# STM32 + ESP32-C3 AT + OneNet 成功案例开发文档

## 目标

本案例实现一个 STM32F103 物联网终端，通过 ESP32-C3 ESP-AT 固件接入 OneNet MQTT，并配套 uni-app 手机端仪表盘。

当前已验证成功的能力：

- AHT20 读取温湿度
- BMP280 读取大气压强
- ADC 读取光照百分比
- OLED 本地显示温度、湿度、压强、光照
- ESP32-C3 通过 ESP-AT 连接 Wi-Fi 与 OneNet MQTT
- OneNet 物模型属性上报成功，返回 `code:200`
- uni-app 显示 `Humi`、`Temp`、`Light`、`Press`、`led`

## 硬件连接

### STM32 与 ESP32-C3 AT 串口

ESP32-C3 使用 Espressif ESP-AT 固件，AT UART 为：

- ESP32-C3 GPIO7 TX -> STM32 PB11 / USART3_RX
- ESP32-C3 GPIO6 RX -> STM32 PB10 / USART3_TX
- ESP32-C3 GND -> STM32 GND

### I2C 传感器与 OLED

STM32 使用 I2C1 remap：

- PB8 -> SCL
- PB9 -> SDA
- 3.3V -> VCC
- GND -> GND

已扫描到的设备地址：

- `0x38` AHT20
- `0x3C` OLED
- `0x77` BMP280

## OneNet 物模型

当前产品：

- Product ID: `45Z51DlJn3`
- Device Name: `stm32-esp32c3`
- MQTT Host: `mqtts.heclouds.com`
- MQTT Port: `1883`

当前代码使用的物模型标识符：

| 功能 | 标识符 | 类型 | 范围 | 步长 | 单位 | 读写 |
|---|---|---|---|---|---|---|
| 湿度 | `Humi` | float | 0-100 | 0.01 | %RH | 读写 |
| 温度 | `Temp` | float | -40-85 | 0.1 | °C | 读写 |
| 光照强度 | `Light` | float | 0-100 | 0.1 | 当前代码为百分比 | 只读 |
| 大气压强 | `Press` | float | 300-1100 | 0.01 | hPa | 读写 |
| PC13_LED | `led` | bool | true/false | - | - | 读写 |

注意：当前 `Light` 代码上报的是光照百分比，不是真实 Lux。如果平台单位保持 Lux，语义不严格；推荐平台单位改成 `%`，或另建 `LightPct`。

## MQTT 主题

```text
$sys/45Z51DlJn3/stm32-esp32c3/thing/property/post
$sys/45Z51DlJn3/stm32-esp32c3/thing/property/post/reply
$sys/45Z51DlJn3/stm32-esp32c3/thing/property/set
$sys/45Z51DlJn3/stm32-esp32c3/thing/property/set_reply
```

## 成功上报方式

### 关键结论

ESP-AT 的 `AT+MQTTPUB` 单条命令长度有限，完整物模型 payload 加 topic 后超过限制会异常或卡住。

本案例必须使用 `AT+MQTTPUBRAW`：

1. 先发送短命令：

```text
AT+MQTTPUBRAW=0,"$sys/45Z51DlJn3/stm32-esp32c3/thing/property/post",152,0,0
```

2. 等待 `>` 提示符。
3. 发送原始 JSON，不再带 AT 字符串转义：

```json
{"id":"2","version":"1.0","params":{"Humi":{"value":47.5},"Temp":{"value":26.7},"led":{"value":false},"Light":{"value":89.4},"Press":{"value":1070.35}}}
```

4. 成功日志：

```text
+MQTTPUB:OK
+MQTTSUBRECV:0,"$sys/45Z51DlJn3/stm32-esp32c3/thing/property/post/reply",37,{"id":"2","code":200,"msg":"success"}
```

## 已解决的关键问题

### 1. FreeRTOS 启动后断言死机

现象：

- 第一次 MQTT 回包或 DMA 中断后程序停止
- J-Link halt 后 PC 在 `vPortValidateInterruptPriority()`

修复：

- `configPRIO_BITS` 设置为 `3`
- `main()` 中 `HAL_Init()` 后调用 `NVIC_SetPriorityGrouping(0)`
- 将 ESP USART3 / DMA1_Channel3 中断优先级降到 `10`

相关文件：

- `User/FreeRTOSConfig.h`
- `User/main.c`
- `User/usart/bsp_usart.c`

### 2. 长 MQTT payload 发送失败

现象：

- `MQTT post command failed`
- 或只打印 `AT+MQTTPUB...`，后续无 `OK`

修复：

- 去掉 `strlen(topic)+strlen(payload)>230` 旧限制
- 将 `AT+MQTTPUB` 改成 `AT+MQTTPUBRAW`
- 发送前把固件中已转义 payload 还原成原始 JSON

相关文件：

- `User/esp8266/bsp_esp8266.c`

### 3. OneNet 返回 Light 步长错误

现象：

```text
{"code":2254,"msg":"float not conform step:identifier:Light"}
```

原因：

- OneNet `Light` 步长为 `0.1`
- 代码上传了 `89.52` 两位小数

修复：

- `Light` 按 `%.1f` 上传

相关文件：

- `APP/esp8266/app_esp8266.c`

### 4. OLED 和传感器 I2C 冲突

现象：

- OLED 能显示，但 AHT20 偶发读失败
- 上传间歇只发光照和 LED

原因：

- OLED、AHT20、BMP280 共用 PB8/PB9 I2C
- OLED 刷屏任务和传感器任务没有互斥

修复：

- 增加 `I2C1BusMutex`
- OLED 刷屏前获取锁
- AHT20/BMP280 读取前获取锁
- OLED 刷新间隔从 100ms 调整到 300ms
- 传感器上报间隔调整到 5s

相关文件：

- `User/freertos.c`
- `APP/oled/app_oled.c`
- `APP/sensor/app_sensor.c`

## uni-app 修改点

App 项目：

```text
E:\Dev\stm32-esp-mqtt\repo\mobile_app_uni
```

已改为读取当前物模型标识符：

- `Humi`
- `Temp`
- `Light`
- `Press`
- `led`

压强模块：

- 图标：`static/pressure.png`
- 页面卡片：`pages/index/index.vue`
- 显示单位：`hPa`
- 显示精度：2 位小数

## 验证流程

1. 烧录 STM32 固件。
2. 打开 J-Link UART，115200 8N1。
3. 观察 I2C 扫描：

```text
0x38
0x3C
0x77
```

4. 观察 Wi-Fi / MQTT：

```text
WIFI CONNECTED
WIFI GOT IP
+MQTTCONNECTED
```

5. 观察上报成功：

```text
AT+MQTTPUBRAW=...
+MQTTPUB:OK
{"code":200,"msg":"success"}
```

6. 打开 OneNet 设备属性页确认数据刷新。
7. 打开 `mobile_app_uni`，确认温度、湿度、光照、压强和 LED 状态显示。

## 常用排查

- 如果卡在 MQTT 发布后，优先检查是否错误使用 `AT+MQTTPUB`。
- 如果 J-Link halt 在 `vPortValidateInterruptPriority()`，检查 NVIC 分组和 UART/DMA 中断优先级。
- 如果 OneNet 返回 `float not conform step`，检查平台步长和代码格式化位数。
- 如果 AHT20 偶发失败，检查 I2C 互斥和 OLED 刷新频率。
- 如果 LED 下发超时，先确认设备订阅了 `thing/property/set`，再确认 `set_reply` 用 `MQTTPUBRAW` 或正确转义发送。
