STM32 + ESP32-C3 Super Mini 项目说明

一、简介
这是一个基于 STM32F103、HAL、FreeRTOS 和 OneNET MQTT 的环境监测项目。
当前默认联网模块已经改成：
- ESP32-C3 Super Mini
- 固件类型：ESP-AT

二、功能
1. 每 2 秒采集一次环境数据
- DHT11：温度、湿度
- 光敏传感器：光照强度

2. 通过 ESP32-C3 Super Mini 联网并接入 OneNET
- 上传温度、湿度、光照、LED 状态
- 接收云端 LED 控制命令

3. OLED 显示页面切换
- 页面 1：湿度当前值 / 最大值 / 最小值
- 页面 2：温度当前值 / 最大值 / 最小值
- 页面 3：光照当前值 / 最大值 / 最小值

4. 本地按键控制 LED

三、硬件接线
1. STM32 和 ESP32-C3 Super Mini
- STM32 PB10 / USART3_TX -> ESP32-C3 GPIO20 / RX
- STM32 PB11 / USART3_RX -> ESP32-C3 GPIO21 / TX
- STM32 GND -> ESP32-C3 GND
- STM32 3.3V -> ESP32-C3 3V3

当前默认不使用硬复位脚：
- ESP_AT_USE_HARD_RESET = 0

2. DHT11
- VCC -> 3.3V
- GND -> GND
- DATA -> PB12

3. 光敏传感器
- VCC -> 3.3V
- GND -> GND
- AO -> PA4
- DO -> 不接

4. OLED(I2C)
- VCC -> 3.3V
- GND -> GND
- SCL -> PB8
- SDA -> PB9

5. 按键
- KEY1 -> PA6
- KEY2 -> PA7
- KEY3 -> PB0

6. LED
- 红灯 -> PA1
- 绿灯 -> PA2
- 蓝灯 -> PA3
- 板载 LED -> PC13

7. 调试串口
- USART1 TX -> PA9
- USART1 RX -> PA10

四、软件配置位置
1. OneNET 和 Wi-Fi 参数
文件：
- APP\esp8266\app_esp8266.c

主要配置项：
- ESP8266_AP_SSID
- ESP8266_AP_PWD
- MQTT_SERVER_IP
- MQTT_SERVER_PORT
- DEVICE_NAME
- PRODUCT_ID
- TOKEN

2. ESP32-C3 串口接线默认配置
文件：
- User\esp8266\bsp_esp_at_port.h

五、推荐阅读文档
1. 接线说明
- Doc\wiring-esp32c3-stm32.md

2. ESP32-C3 替换说明
- Doc\esp32c3-supermini-migration.md

六、使用步骤
1. 按接线说明连接 STM32、ESP32-C3 和传感器
2. 给 ESP32-C3 刷 ESP-AT 固件
3. 修改 Wi-Fi 和 OneNET 参数
4. 编译下载 STM32 固件
5. 打开串口观察调试信息

七、注意事项
1. 传感器不要接到 ESP32-C3
传感器仍然接 STM32，ESP32-C3 只负责联网和 MQTT。

2. ESP32-C3 使用 3.3V 供电和 3.3V 串口电平

3. 如果使用手机端页面，建议打开整理后的项目：
- ..\..\mobile_app_uni
