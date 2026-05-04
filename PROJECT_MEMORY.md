# stm32-esp-mqtt 项目记忆

> 这是当前已调通项目的开发上下文。以后继续开发时，优先从这个文件和 `repo/STM32_ESP32C3_OneNet_Development_Guide.md` 读取信息。

## 正确项目目录

- 项目总目录：`E:\Dev\stm32-esp-mqtt`
- Git 仓库目录：`E:\Dev\stm32-esp-mqtt\repo`
- STM32 固件：`E:\Dev\stm32-esp-mqtt\repo\iot_terminal`
- uni-app 手机端：`E:\Dev\stm32-esp-mqtt\repo\mobile_app_uni`
- 成功案例开发文档：`E:\Dev\stm32-esp-mqtt\repo\STM32_ESP32C3_OneNet_Development_Guide.md`
- 可复用 Codex 技能：`C:\Users\13158\.codex\skills\stm32-esp32c3-onenet-terminal`

## 当前已调通的系统

STM32F103 通过 ESP32-C3 ESP-AT 接入 OneNet MQTT，读取 AHT20、BMP280、光照 ADC，并在 OLED 和手机 App 上显示。

已验证成功：

- Wi-Fi：`CMCC-etru` / `4us46hfx`
- OneNet MQTT：连接成功
- 属性上报：返回 `code:200`
- OLED：显示温度、湿度、压强、光照
- App：已添加压强模块，读取当前物模型标识符

## 硬件接线

### ESP32-C3 ESP-AT UART

- ESP32-C3 GPIO7 TX -> STM32 PB11 / USART3_RX
- ESP32-C3 GPIO6 RX -> STM32 PB10 / USART3_TX
- GND 共地

### I2C 传感器/OLED

- PB8 -> SCL
- PB9 -> SDA
- 3.3V -> VCC
- GND -> GND

已扫描到：

- `0x38` AHT20
- `0x3C` OLED
- `0x77` BMP280

## OneNet 物模型

产品和设备：

- Product ID：`45Z51DlJn3`
- Device Name：`stm32-esp32c3`
- MQTT Host：`mqtts.heclouds.com`
- MQTT Port：`1883`

标识符：

| 标识符 | 含义 | 类型 | 备注 |
|---|---|---|---|
| `Humi` | 湿度 | float | `%RH` |
| `Temp` | 温度 | float | `°C` |
| `Light` | 光照 | float | 当前代码为百分比，按 1 位小数上传 |
| `Press` | 大气压强 | float | `hPa`，2 位小数 |
| `led` | PC13 LED | bool | `true/false` |

不要再使用旧标识符：`EnvironmentHumidity`、`EnvironmentTemperature`、`LightLux`。

## 关键技术结论

### MQTT 上报必须用 `AT+MQTTPUBRAW`

完整 OneNet property JSON + topic 超过 `AT+MQTTPUB` 单行命令限制，会卡住或失败。

成功方式：

1. 发送 `AT+MQTTPUBRAW=0,"topic",payload_len,0,0`
2. 等待 `>`
3. 发送原始 JSON
4. 等待：
   - `+MQTTPUB:OK`
   - `thing/property/post/reply` 中 `code:200`

### FreeRTOS 中断优先级

如果程序卡死，J-Link halt 后 PC 在 `vPortValidateInterruptPriority()`，说明有中断优先级不合法。

当前修复：

- `User/FreeRTOSConfig.h`：`configPRIO_BITS = 3`
- `User/main.c`：`HAL_Init()` 后调用 `NVIC_SetPriorityGrouping(0)`
- `User/usart/bsp_usart.c`：`DMA1_Channel3_IRQn` 和 `USART3_IRQn` 优先级为 `10`

### I2C 互斥

OLED、AHT20、BMP280 共用 PB8/PB9。必须使用 `I2C1BusMutex`，否则 AHT20/BMP280 会偶发读取失败。

相关文件：

- `User/freertos.c`
- `APP/oled/app_oled.c`
- `APP/sensor/app_sensor.c`

### OneNet 步长

`Light` 平台步长是 `0.1`，代码必须按 `%.1f` 上报，否则 OneNet 返回：

```text
float not conform step:identifier:Light
```

## 手机 App 当前状态

App 目录：`repo/mobile_app_uni`

已改内容：

- 首页读取 `Humi`、`Temp`、`Light`、`Press`、`led`
- 新增压强卡片
- 新增图标：`static/pressure.png`
- `Light` 显示 1 位小数
- `Press` 显示 2 位小数，单位 `hPa`

运行方式：用 HBuilderX 打开 `repo/mobile_app_uni`。

## 以后继续开发时的建议入口

1. 先打开：`E:\Dev\stm32-esp-mqtt\repo`
2. 如果调 STM32：打开 `repo/iot_terminal/Project/IoT_Terminal.uvprojx`
3. 如果调 App：打开 `repo/mobile_app_uni`
4. 如果让 Codex 继续：明确说使用 `$stm32-esp32c3-onenet-terminal`

## 最后一次成功日志特征

```text
AT+MQTTPUBRAW=0,"$sys/45Z51DlJn3/stm32-esp32c3/thing/property/post",152,0,0
OK
>
+MQTTPUB:OK
+MQTTSUBRECV:0,"$sys/45Z51DlJn3/stm32-esp32c3/thing/property/post/reply",37,{"id":"2","code":200,"msg":"success"}
```
