# 传感器扩展指南

## 扩展原则

ESP32-C3 只承担 Wi-Fi/MQTT AT 从模块职责。新增传感器时，不改 ESP32-C3 AT 固件，不改基础刷机包，优先在 STM32 侧扩展。

## 推荐流程

1. 先确认传感器接口：I2C、ADC、UART、SPI 或 GPIO。
2. 检查引脚是否和 USART3、I2C1、OLED、按键、LED 冲突。
3. 在 STM32 工程中新增或扩展采集驱动。
4. 把采集值写入统一显示/上报数据结构。
5. 扩展 OLED 页面，而不是继续挤压 P1 首页。
6. 在 OneNet 物模型中新增字段，字段名和代码常量保持一致。
7. 修改 MQTT property post JSON，继续使用 `AT+MQTTPUBRAW`。
8. 修改 uni-app 页面展示新字段。

## I2C 设备

当前 PB8/PB9 已挂载：

- AHT20：`0x38`
- OLED：`0x3C`
- BMP280：`0x77`

新增 I2C 传感器必须：

- 确认地址不冲突。
- 使用 `I2C1BusMutex` 保护访问。
- 避免在 OLED 刷新高频期间长时间占用总线。

## UART 设备

USART3 已固定给 ESP32-C3 AT 模块。新增 UART 传感器不要复用 USART3，优先选其他串口或改用 I2C/SPI/ADC 版本。

## C6T6 裁剪建议

如果后续改成 STM32F103C6T6，先做 lite profile：

- 减少 OLED 页面数量。
- 减少串口日志字符串。
- 缩小 MQTT 队列和缓冲区。
- 只保留必要传感器。
- 先编译看 Flash/RAM，再决定保留哪些功能。
