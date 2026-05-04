# ESP32-C3 Super Mini 替换说明

## 结论

这个项目可以从 `ESP8266 / ESP-01` 迁移到 `ESP32-C3 Super Mini`，但前提不是刷普通 Arduino 固件，而是给 `ESP32-C3` 刷入 `ESP-AT` 固件。

这样做以后：

- STM32 侧整体架构可以保留
- FreeRTOS 任务可以保留
- MQTT 主题和 OneNET 逻辑可以保留
- 主要变化集中在联网模组和接线方案

## 当前项目是怎么工作的

这个工程不是 STM32 自己跑完整 MQTT 协议栈，而是：

1. STM32 采集传感器数据
2. STM32 通过 UART 给外部模组发送 `AT` 指令
3. 外部模组负责 Wi-Fi 联网和 MQTT 通信

所以能不能替换，关键不在于是 `ESP8266` 还是 `ESP32-C3`，而在于新模组能不能提供兼容的：

- UART AT 指令通信
- Wi-Fi STA 联网
- MQTT AT 指令
- 订阅和属性上报

## 为什么 ESP32-C3 Super Mini 可以替代

只要这块板子刷的是 `ESP-AT`，并且 AT 串口接对了，它就可以继续扮演“串口联网模组”的角色。

替换以后仍然是：

- STM32 采集数据
- ESP32-C3 负责联网
- OneNET 负责云端收发属性

## 当前工程默认接线

当前默认配置如下：

- `STM32 PB10 / USART3_TX -> ESP32-C3 GPIO20 / RX`
- `STM32 PB11 / USART3_RX -> ESP32-C3 GPIO21 / TX`
- `STM32 GND -> ESP32-C3 GND`
- `STM32 3.3V -> ESP32-C3 3V3`

并且默认：

- 波特率：`115200`
- 不依赖硬复位脚
- 继续使用 `USART3 + DMA`

对应配置文件：

- [bsp_esp_at_port.h](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/esp8266/bsp_esp_at_port.h)

## 已经做过的代码适配

### 1. 增加了模组适配层

文件：

- [bsp_esp_at_port.h](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/esp8266/bsp_esp_at_port.h)

作用：

- 抽出波特率
- 抽出复位脚配置
- 抽出是否启用硬复位
- 默认切换到 `ESP32-C3 Super Mini`

### 2. 修正了底层初始化顺序

文件：

- [bsp_esp8266.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/esp8266/bsp_esp8266.c)

作用：

- 先初始化 GPIO 和 UART
- 再执行复位
- 避免复位脚在未配置前就被访问

### 3. 串口波特率改为读取统一配置

文件：

- [bsp_usart.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/usart/bsp_usart.c)

作用：

- `USART3` 默认波特率由 `ESP_AT_UART_BAUDRATE` 控制

## 后面如果继续换板，优先改哪里

优先修改这个文件：

- [bsp_esp_at_port.h](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/esp8266/bsp_esp_at_port.h)

主要配置项：

- `ESP_AT_UART_BAUDRATE`
- `ESP_AT_USE_HARD_RESET`
- `ESP_AT_RST_GPIO_PORT`
- `ESP_AT_RST_PIN`

当前默认值：

```c
#define ESP_AT_USE_HARD_RESET 0
```

## 风险点

### 1. 不是所有 ESP32-C3 Super Mini 到手就能直接跑

很多板子默认并不是 `ESP-AT` 固件，所以直接接 STM32 不会响应当前工程里的 `AT`、`AT+CWJAP`、`AT+MQTTCONN`。

### 2. USB 下载口不等于 AT 串口

不要默认认为板子上带 USB 的那组串口就是 STM32 要接的 `AT` 串口。

### 3. 传感器不要接到 ESP32-C3

当前工程里：

- 传感器接 STM32
- ESP32-C3 只负责联网

## 推荐迁移顺序

1. 先保持 STM32 业务逻辑不动
2. 给 `ESP32-C3` 刷入 `ESP-AT`
3. 按当前默认接线接好串口
4. 先验证这些 AT 指令：

- `AT`
- `AT+GMR`
- `AT+CWMODE=1`
- `AT+CWJAP=...`

5. 再验证固件中的：

- `ESP8266_Test()`
- Wi-Fi 连接
- MQTT 连接
- 属性上报

## 相关文档

- [完整接线说明](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/Doc/wiring-esp32c3-stm32.md)
- [仓库 README](/E:/Dev/stm32-esp-mqtt/repo/README.md)
