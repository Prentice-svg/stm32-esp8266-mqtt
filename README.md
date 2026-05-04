# stm32-esp32c3-mqtt

这是一个基于 `STM32F103 + HAL + FreeRTOS + OneNET MQTT` 的环境监测项目。

当前仓库已经按 `ESP32-C3 Super Mini + ESP-AT` 的方案整理，不再默认使用 `ESP-01 / ESP8266` 作为联网模块。

## 项目结构

- [物联网终端](/E:/Dev/stm32-esp-mqtt/repo/物联网终端)
  STM32 固件工程
- [手机APP](/E:/Dev/stm32-esp-mqtt/repo/手机APP)
  原始 `uni-app` 工程
- [mobile_app_uni](/E:/Dev/stm32-esp-mqtt/repo/mobile_app_uni)
  整理后的可继续维护版本
- [iot_terminal](/E:/Dev/stm32-esp-mqtt/repo/iot_terminal)
  纯英文路径的 STM32 工程副本，便于 Keil 打开

## 功能

1. 每 2 秒采集一次环境数据

- DHT11：温度、湿度
- 光敏传感器：光照强度

2. 通过 `ESP32-C3 Super Mini` 联网并接入 OneNET

- 上传温度、湿度、光照、LED 状态
- 接收云端下发的 LED 控制命令

3. OLED 页面切换显示

- 页面 1：湿度当前值 / 最大值 / 最小值
- 页面 2：温度当前值 / 最大值 / 最小值
- 页面 3：光照当前值 / 最大值 / 最小值

4. 本地按键控制板载 LED

5. 串口输出调试信息

## 当前默认硬件方案

### STM32 和 ESP32-C3 Super Mini

- `STM32 PB10 / USART3_TX -> ESP32-C3 GPIO20 / RX`
- `STM32 PB11 / USART3_RX -> ESP32-C3 GPIO21 / TX`
- `STM32 GND -> ESP32-C3 GND`
- `STM32 3.3V -> ESP32-C3 3V3`

说明：

- 当前工程默认 `ESP_AT_USE_HARD_RESET = 0`
- 也就是默认不依赖 STM32 控制 ESP32-C3 的 `RST/EN`
- 串口波特率默认 `115200`

对应代码配置文件：

- [bsp_esp_at_port.h](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/esp8266/bsp_esp_at_port.h)
- [bsp_usart.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/usart/bsp_usart.c)

### 传感器和外设接 STM32

#### DHT11

- `VCC -> 3.3V`
- `GND -> GND`
- `DATA -> PB12`

对应代码：

- [bsp_dht11.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/dht11/bsp_dht11.c)

#### 光敏传感器

- `VCC -> 3.3V`
- `GND -> GND`
- `AO -> PA4`
- `DO -> 不接`

对应代码：

- [bsp_adc.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/adc/bsp_adc.c)

#### OLED(I2C)

- `VCC -> 3.3V`
- `GND -> GND`
- `SCL -> PB8`
- `SDA -> PB9`

对应代码：

- [bsp_i2c.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/i2c/bsp_i2c.c)

#### 按键

- `KEY1 -> PA6`
- `KEY2 -> PA7`
- `KEY3 -> PB0`

对应代码：

- [bsp_key.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/key/bsp_key.c)

#### LED

- `红灯 -> PA1`
- `绿灯 -> PA2`
- `蓝灯 -> PA3`
- `板载 LED -> PC13`

对应代码：

- [bsp_led.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/led/bsp_led.c)

## 项目原理

这个项目不是让 STM32 自己跑完整的 MQTT 协议栈，而是：

1. STM32 采集传感器数据
2. STM32 通过串口给外部联网模组发送 `AT` 指令
3. 外部模组负责 Wi-Fi 联网和 MQTT 通信

因此当前迁移到 `ESP32-C3 Super Mini` 的前提是：

- 给 ESP32-C3 刷入 `ESP-AT` 固件
- 串口接线正确
- 继续按“STM32 主控 + ESP32-C3 AT 联网模组”的架构工作

## 关键配置位置

### OneNET 和 Wi-Fi 参数

文件：

- [app_esp8266.c](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/APP/esp8266/app_esp8266.c)

主要配置项：

- `ESP8266_AP_SSID`
- `ESP8266_AP_PWD`
- `MQTT_SERVER_IP`
- `MQTT_SERVER_PORT`
- `DEVICE_NAME`
- `PRODUCT_ID`
- `TOKEN`

### ESP32-C3 Super Mini 串口配置

文件：

- [bsp_esp_at_port.h](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/User/esp8266/bsp_esp_at_port.h)

主要配置项：

- `ESP_AT_MODULE_NAME`
- `ESP_AT_UART_BAUDRATE`
- `ESP_AT_USE_HARD_RESET`
- `ESP_AT_RST_GPIO_PORT`
- `ESP_AT_RST_PIN`

## 推荐先看这两份文档

- [ESP32-C3 替换说明](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/Doc/esp32c3-supermini-migration.md)
- [完整接线说明](/E:/Dev/stm32-esp-mqtt/repo/物联网终端/Doc/wiring-esp32c3-stm32.md)

## 使用步骤

1. 按接线说明连接 STM32、ESP32-C3、DHT11、光敏、OLED、按键和 LED
2. 给 `ESP32-C3 Super Mini` 刷入 `ESP-AT` 固件
3. 修改 Wi-Fi 和 OneNET 参数
4. 编译并下载 STM32 固件
5. 打开串口观察联网和 MQTT 调试信息

## 注意事项

1. 传感器不要接到 `ESP32-C3`

传感器仍然接 `STM32`，`ESP32-C3` 只负责 `Wi-Fi + MQTT + AT 串口通信`。

2. `ESP32-C3` 只能使用 `3.3V` 供电和 `3.3V` 串口电平

不要把 STM32 的 `5V` 直接接到 `ESP32-C3` 的 IO。
