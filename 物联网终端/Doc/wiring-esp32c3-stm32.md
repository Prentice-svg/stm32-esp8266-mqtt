# STM32 + ESP32-C3 Super Mini 接线说明

## 先说结论

这个项目里：

- `ESP32-C3 Super Mini` 只负责 `Wi-Fi + MQTT + AT 串口通信`
- `DHT11 / 光敏 / OLED / 按键 / LED` 这些外设仍然接在 `STM32`

也就是说：

- 传感器不是接到 `ESP32-C3`
- 传感器还是接 `STM32`
- STM32 采集数据后，再通过串口发给 `ESP32-C3`

## 1. STM32 和 ESP32-C3 Super Mini 的接线

按当前工程默认配置：

- `STM32 PB10 / USART3_TX -> ESP32-C3 GPIO20 / RX`
- `STM32 PB11 / USART3_RX -> ESP32-C3 GPIO21 / TX`
- `STM32 GND -> ESP32-C3 GND`
- `STM32 3.3V -> ESP32-C3 3V3`

注意：

- 串口一定要交叉接
- 只能接 `3.3V`
- 不要把 STM32 的 `5V` 直接接到 `ESP32-C3` 的 IO

当前工程默认不依赖硬复位脚：

- `ESP_AT_USE_HARD_RESET = 0`

所以可以先不接 `EN/RST`

## 2. 传感器和显示模块接 STM32 的方式

### DHT11

- `VCC -> 3.3V`
- `GND -> GND`
- `DATA -> STM32 PB12`

代码来源：

- [bsp_dht11.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/dht11/bsp_dht11.c)

### 光敏传感器

- `VCC -> 3.3V`
- `GND -> GND`
- `AO -> STM32 PA4`
- `DO -> 不接`

代码来源：

- [bsp_adc.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/adc/bsp_adc.c)

### OLED(I2C)

- `VCC -> 3.3V`
- `GND -> GND`
- `SCL -> STM32 PB8`
- `SDA -> STM32 PB9`

代码来源：

- [bsp_i2c.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/i2c/bsp_i2c.c)

### 按键

- `KEY1 -> STM32 PA6`
- `KEY2 -> STM32 PA7`
- `KEY3 -> STM32 PB0`

代码来源：

- [bsp_key.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/key/bsp_key.c)

### LED

- `红灯 -> STM32 PA1`
- `绿灯 -> STM32 PA2`
- `蓝灯 -> STM32 PA3`
- `板载 LED -> STM32 PC13`

代码来源：

- [bsp_led.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/led/bsp_led.c)

## 3. 整个系统的接线关系

### 传感器链路

- `DHT11 -> STM32`
- `光敏传感器 -> STM32`
- `OLED -> STM32`
- `按键 -> STM32`
- `LED -> STM32`

### 通信链路

- `STM32 USART3 -> ESP32-C3 Super Mini`
- `ESP32-C3 Super Mini -> Wi-Fi -> OneNET`

## 4. 关键代码位置

- ESP32-C3 串口连接说明：
  [bsp_esp_at_port.h](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/esp8266/bsp_esp_at_port.h)
- STM32 串口引脚：
  [bsp_usart.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/usart/bsp_usart.c)
- DHT11 引脚：
  [bsp_dht11.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/dht11/bsp_dht11.c)
- 光敏 ADC 引脚：
  [bsp_adc.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/adc/bsp_adc.c)
- OLED I2C 引脚：
  [bsp_i2c.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/i2c/bsp_i2c.c)
- 按键引脚：
  [bsp_key.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/key/bsp_key.c)
- LED 引脚：
  [bsp_led.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/led/bsp_led.c)

## 5. 推荐同时查看的文档

- [仓库 README](/E:/Dev/stm32-esp-mqtt/repo/README.md)
- [ESP32-C3 替换说明](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/Doc/esp32c3-supermini-migration.md)
