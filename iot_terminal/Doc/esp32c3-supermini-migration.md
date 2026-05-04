# ESP32-C3 Super Mini 替换说明

## 结论

这个项目可以从 `ESP8266/ESP-01` 替换为 `ESP32-C3 Super Mini`，前提不是直接刷 Arduino 或普通示例固件，而是给 ESP32-C3 刷 `ESP-AT` 固件，让它继续扮演当前工程里的“串口 AT Wi-Fi/MQTT 从模块”。

这样做时，STM32 侧的整体架构不用推翻：

- `User/esp8266/bsp_esp8266.c` 仍然负责发送/接收 AT 指令
- `APP/esp8266/app_esp8266.c` 仍然负责配网、连 MQTT、订阅/发布
- `FreeRTOS` 的打包、发送、接收、响应任务都可以保留

## 当前项目是怎么实现的

### 1. 初始化阶段

`main.c` 里先完成板级初始化，然后循环调用 `ESP8266_Preparation()`，直到 Wi-Fi 和 MQTT 都准备好。

对应代码：

- `User/main.c`
- `APP/esp8266/app_esp8266.c`

初始化流程是：

1. 初始化串口和 DMA
2. 软件复位模块
3. `AT` 测试
4. `AT+CWMODE=1` 切到 STA
5. `AT+CWJAP` 连接 Wi-Fi
6. `AT+MQTTUSERCFG` 配置 OneNet 鉴权
7. `AT+MQTTCONN` 连接 MQTT 服务器
8. `AT+MQTTSUB` 订阅 `post/reply`
9. `AT+MQTTSUB` 订阅 `set`

### 2. 数据上报

`StartSensorTask()` 每 2 秒采一次传感器数据，送到两个队列：

- 一个给 OLED/LED 显示逻辑
- 一个给 MQTT 打包逻辑

`StartESP8266PackTask()` 会拼出 OneNet 属性上报 JSON，再送到发送队列。

`StartESP8266SendTask()` 最终调用 `ESP8266_MQTTPublish()` 发出：

- 主题：`$sys/{product_id}/{device_name}/thing/property/post`
- 载荷：温湿度、光照、LED 状态

### 3. 云端控制

`StartESP8266ReceiveTask()` 监听串口收到的 `+MQTTSUBRECV`。

`StartESP8266RespondTask()` 从订阅消息里解析 JSON：

- 发现 `"LED":true` 就开灯
- 发现 `"LED":false` 就关灯
- 然后回发 `set_reply`

## 为什么 ESP32-C3 可以替代

这个工程依赖的不是 ESP8266 芯片本身，而是这几个能力：

- UART AT 指令通信
- Wi-Fi STA 联网
- MQTT AT 指令
- 订阅消息回推 `+MQTTSUBRECV`

只要 `ESP32-C3 Super Mini` 刷的是 `ESP-AT` 固件，并且使用兼容的 MQTT AT 指令集，它就可以替代原来的 ESP8266 模组。

## 当前工程默认接线

现在工程默认就是按你这块 `ESP32-C3 Super Mini` 来配的：

- `STM32 PB10 / USART3_TX -> ESP32-C3 GPIO20 / RX`
- `STM32 PB11 / USART3_RX -> ESP32-C3 GPIO21 / TX`
- `STM32 GND -> ESP32-C3 GND`
- `STM32 3.3V -> ESP32-C3 3V3`

并且默认：

- 使用 `115200` 波特率
- 不依赖 STM32 控制 ESP32-C3 的硬复位脚
- 继续使用 `USART3 + DMA` 收发 AT 数据

## 已经做的代码适配

为了后续换模组更顺一点，这次已经在工程里做了两点整理：

1. 增加 `User/esp8266/bsp_esp_at_port.h`
   - 把 AT 模组的波特率、复位脚、是否启用硬复位抽成配置项
   - 默认值已切换为 `ESP32-C3 Super Mini`
2. 修正 `ESP8266_Init()` 初始化顺序
   - 先初始化 GPIO/UART，再执行硬复位，避免复位脚在未配置前被操作

## 你后面换成 ESP32-C3 时要改什么

### STM32 代码侧

如果你后面还要换板，优先只改这一个配置文件：

- `User/esp8266/bsp_esp_at_port.h`

通常先改这些项：

- `ESP_AT_USE_HARD_RESET`
- `ESP_AT_RST_GPIO_PORT`
- `ESP_AT_RST_PIN`
- `ESP_AT_UART_BAUDRATE`

当前工程已经按这个板型默认设置为：

```c
#define ESP_AT_USE_HARD_RESET 0
```

### 硬件接线侧

当前项目原始接法是：

- `PB10 -> ESP TX`
- `PB11 -> ESP RX`
- `PB13 -> ESP RST`

换成 ESP32-C3 时，本质上还是 TTL 串口交叉连接：

- `STM32 TX -> ESP32-C3 RX`
- `STM32 RX -> ESP32-C3 TX`
- `GND -> GND`
- `3.3V -> 3.3V`

如果接硬复位，再多接一根：

- `STM32 GPIO -> ESP32-C3 EN/RST`

注意：

- 要确认你买的 `ESP32-C3 Super Mini` 是否把你需要的 UART 引脚真正引出来了
- 不同小板丝印命名可能不同，别只看 “TX/RX” 字样就直接下结论
- 如果板子默认把 USB 串口占给下载/日志口，AT 口不一定就是那组 USB 转串口

## 风险点

### 1. 不是所有 “ESP32-C3 Super Mini” 到手即 AT

很多成品板默认不是 `ESP-AT` 固件，所以直接接 STM32 后，这个项目的 `AT`、`AT+CWJAP`、`AT+MQTTCONN` 可能完全不响应。先刷 `ESP-AT`。

### 2. AT 口和下载口可能不是同一组 UART

ESP32-C3 的官方 `ESP-AT` 默认：

- `UART0` 用于下载/日志
- `UART1` 用于 AT 命令

因此你不能默认认为 USB 口连着的那组串口就是 STM32 应该接的 AT 口。

### 3. 主题和 Token 长度要继续留意

当前工程里 `AT+MQTTPUB` 是拼成一整条字符串发出的，长度仍受 AT 命令长度限制。OneNet 的 topic 和 token 如果以后继续变长，可能需要切到 `AT+MQTTPUBRAW` 方案。

## 推荐迁移顺序

1. 先保持 STM32 工程不动
2. 给 ESP32-C3 Super Mini 刷 `ESP-AT`
3. 只做最小接线验证：
   - `AT`
   - `AT+GMR`
   - `AT+CWMODE=1`
   - `AT+CWJAP=...`
4. STM32 里先只验证 `ESP8266_Test()`
5. 再验证 Wi-Fi 连接
6. 最后验证 MQTT 连接和属性上报

## 如果你下一步要继续

下一轮最值得做的是：

1. 我直接帮你把这个工程再往前改一版，做成 `ESP8266 / ESP32-C3` 双配置切换
2. 或者我按你手上的 `ESP32-C3 Super Mini` 实物引脚图，直接替你改成对应接线版本
