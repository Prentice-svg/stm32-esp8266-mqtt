# STM32F103 + ESP32-C3 Super Mini AT 基准开发指南

## 目标

这份基准用于把 ESP32-C3 Super Mini 固定成 STM32F103 的 Wi-Fi/MQTT AT 从模块。后续新增传感器时，优先扩展 STM32 采集、显示和 OneNet 物模型，不重复处理 ESP32-C3 固件和 AT 基础链路。

## 标准硬件

- 标准 MCU：STM32F103C8T6。
- 低成本备选：STM32F103C6T6，只作为后续 lite profile，不直接承诺当前完整工程无裁剪运行。
- Wi-Fi 模块：ESP32-C3 Super Mini，刷 Espressif ESP-AT。
- ESP-AT 固件：`ESP32-C3-MINI-1-AT-V4.1.1.0`。

## 已验证接线

| STM32F103 | ESP32-C3 Super Mini | 说明 |
|---|---|---|
| PB10 / USART3_TX | GPIO6 / RX | STM32 发 AT 命令到 ESP32-C3 |
| PB11 / USART3_RX | GPIO7 / TX | STM32 接收 ESP32-C3 AT 回包 |
| GND | GND | 必须共地 |
| 3.3V | 3V3 | 使用稳定 3.3V 供电 |

不要把 ESP32-C3 的 USB 下载串口默认当成 AT 命令口。官方 ESP32-C3-MINI-1 AT 固件的 AT 口是 GPIO7/GPIO6。

## STM32 工程边界

当前工程入口：

- 固件工程：`iot_terminal/Project/IoT_Terminal.uvprojx`
- AT 端口配置：`iot_terminal/User/esp8266/bsp_esp_at_port.h`
- AT 驱动：`iot_terminal/User/esp8266/bsp_esp8266.c`
- OneNet 业务：`iot_terminal/APP/esp8266/app_esp8266.c`

必须保留的基准规则：

- USART3 波特率默认 `115200`。
- 使用 `AT+MQTTPUBRAW` 发送完整 OneNet property post。
- FreeRTOS `configPRIO_BITS` 保持 `3`。
- `HAL_Init()` 后保持 `NVIC_SetPriorityGrouping(0)`。
- USART3 和 DMA1_Channel3 IRQ 优先级保持 FreeRTOS-safe，当前工作值为 `10`。
- OLED、AHT20、BMP280 共用 I2C1 时必须使用 `I2C1BusMutex`。

## OneNet 基准物模型

当前基准使用：

| Identifier | 说明 | 格式 |
|---|---|---|
| `Humi` | 湿度 | float |
| `Temp` | 温度 | float，1 位小数 |
| `Light` | 光照百分比 | float，1 位小数 |
| `Press` | 大气压强 | float，2 位小数，hPa |
| `led` | PC13 LED | bool |

不要再使用旧标识符 `EnvironmentHumidity`、`EnvironmentTemperature`、`LightLux`。

## 新传感器扩展规则

新增传感器时按这个顺序做：

1. 确认硬件接口，避免和 PB8/PB9 I2C、USART3、PC13 LED 冲突。
2. 在传感器任务里采集数据，必要时用互斥保护共享总线。
3. 扩展显示数据结构和 OLED 页面。
4. 扩展 OneNet 物模型字段和上报 JSON。
5. 更新 uni-app 页面。
6. 更新本文档和 OneNet 字段表。

C6T6 成本版本需要单独评估 Flash/RAM，优先裁剪日志字符串、OLED 页面、队列深度和非必要传感器。
