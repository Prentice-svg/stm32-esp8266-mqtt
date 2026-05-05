# ESP32-C3 Super Mini 迁移说明

这份旧迁移说明已整理为新的可复用基准包：

```text
baseline/esp32c3_at/
```

以后以新基准包中的文档为准：

- `baseline/esp32c3_at/docs/STM32F103_ESP32C3_AT_Baseline_Guide.md`
- `baseline/esp32c3_at/docs/ESP32C3_AT_Flashing_Guide.md`
- `baseline/esp32c3_at/docs/Sensor_Extension_Guide.md`

当前已验证接线：

| STM32F103 | ESP32-C3 Super Mini |
|---|---|
| PB10 / USART3_TX | GPIO6 / RX |
| PB11 / USART3_RX | GPIO7 / TX |
| GND | GND |
| 3.3V | 3V3 |

注意：不要沿用旧资料中的 GPIO20/GPIO21 描述。当前基准使用官方 ESP32-C3-MINI-1 ESP-AT 固件，AT UART 为 GPIO7/GPIO6。
