一、简介
1. 这是一个基于 HAL 库 + FreeRTOS + MQTT 的 STM32F103 环境监测系统，通过 ESP32-C3 Super Mini 的 ESP-AT 固件与 MQTT 服务器（OneNET）通信。

二、功能
1. 每 2 秒采集一次：
(1) DHT11：温度、湿度
(2) 光敏传感器：光照

2. ESP32-C3 与 MQTT 服务器通信：
(1) 上报湿度、温度、光照强度、板载 LED 状态到 MQTT 服务器
(2) 接收 MQTT 服务器下发的指令，控制板载 LED 亮灭

3. 按键 1 切换 OLED 显示画面：
(1) 画面 1：湿度（当前 / 最大 / 最小）
(2) 画面 2：温度（当前 / 最大 / 最小）
(3) 画面 3：光照强度（当前 / 最大 / 最小）

4. 按键 2 控制板载 LED 亮灭。

5. LED 报警：
(1) 湿度 > 阈值：红灯亮
(2) 温度 > 阈值：绿灯亮
(3) 光照强度 < 阈值：蓝灯亮

6. 串口（CH340）输出调试信息。

三、硬件接线
1. DHT11：
VCC -> 3.3V
GND -> GND
DATA -> PB12

2. 光敏传感器：
VCC -> 3.3V
GND -> GND
AO -> PA4
DO -> 悬空

3. ESP32-C3 Super Mini（必须先刷 ESP-AT 固件）：
3V3 -> STM32 3.3V
GND -> STM32 GND
GPIO6 / RX -> STM32 PB10 / USART3_TX
GPIO7 / TX -> STM32 PB11 / USART3_RX

默认不接 ESP32-C3 EN/RST，工程使用 AT+RST 软件复位。
官方 ESP32-C3-MINI-1 AT 固件默认启用 UART 硬件流控，本工程启动后会发送 AT+UART_CUR=115200,8,1,0,0 临时关闭流控，因此正常使用只需要 3V3/GND/TX/RX。

4. 按键 1：
一端 -> 3.3V
另一端 -> PA6

5. 按键 2：
一端 -> 3.3V
另一端 -> PA7

6. 0.96 寸 OLED：
VDD -> 3.3V
GND -> GND
SCL/SCK -> PB8
SDA -> PB9

7. LED：
红灯正极 -> 3.3V，负极 -> PA1
绿灯正极 -> 3.3V，负极 -> PA2
蓝灯正极 -> 3.3V，负极 -> PA3

8. USB 转串口 CH340：
VCC -> 3.3V
GND -> GND
TXD -> PA10 / USART1_RX
RXD -> PA9 / USART1_TX

四、软件配置
1. ESP32-C3 AT 模块配置：
User/esp8266/bsp_esp_at_port.h
- ESP_AT_MODULE_NAME
- ESP_AT_UART_BAUDRATE
- ESP_AT_USE_HARD_RESET

2. Wi-Fi 和 MQTT 服务器：
APP/esp8266/app_esp8266.c
- ESP_AT_AP_SSID：Wi-Fi 名称
- ESP_AT_AP_PWD：Wi-Fi 密码
- MQTT_SERVER_IP：MQTT 服务器地址
- MQTT_SERVER_PORT：MQTT 服务器端口
- DEVICE_NAME：OneNET 设备名
- PRODUCT_ID：OneNET 产品 ID
- TOKEN：OneNET token
- MQTT_TOPIC_POST：属性上报主题
- MQTT_TOPIC_POST_REPLY：属性上报回复主题
- MQTT_TOPIC_SET：属性设置主题
- MQTT_TOPIC_SET_REPLY：属性设置回复主题

3. 报警阈值：
APP/led/app_led.c

五、注意事项
1. ESP32-C3 必须刷 ESP-AT 固件，普通 Arduino/示例固件不会响应 AT 指令。
2. 当前工程使用 USART3：PB10 为 STM32 TX，PB11 为 STM32 RX，和 ESP32-C3 串口必须交叉连接。
3. 当前刷入的是 Espressif 官方 ESP32-C3-MINI-1 AT 固件，默认 AT 串口是 UART1：GPIO7 为 TX，GPIO6 为 RX。USB COM5 主要用于下载/日志，不是 STM32 工程使用的 AT 口。
4. ESP32-C3 和 STM32 必须共地。
5. 如果 ESP32-C3 独立 USB 供电，STM32 的 3.3V 可以不接到 ESP32-C3，只保留 GND/TX/RX。
