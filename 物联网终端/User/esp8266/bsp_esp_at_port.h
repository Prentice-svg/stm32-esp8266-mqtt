#ifndef __BSP_ESP_AT_PORT_H
#define __BSP_ESP_AT_PORT_H

/*
 * Default profile for this project after migration to ESP32-C3 Super Mini.
 *
 * Flash ESP-AT firmware to the ESP32-C3, then wire:
 *   - STM32 PB10 / USART3_TX -> ESP32-C3 GPIO20 / RX
 *   - STM32 PB11 / USART3_RX -> ESP32-C3 GPIO21 / TX
 *   - STM32 GND             -> ESP32-C3 GND
 *   - STM32 3.3V            -> ESP32-C3 3V3
 *
 * This board image only exposes RX/TX clearly, so the project defaults to
 * software reset and does not require a dedicated STM32 -> ESP32-C3 reset wire.
 */

#define ESP_AT_MODULE_NAME            "ESP32-C3 Super Mini"

#define ESP_AT_UART_BAUDRATE          115200

#define ESP_AT_USE_HARD_RESET         0
#define ESP_AT_RST_GPIO_PORT          GPIOB
#define ESP_AT_RST_PIN                GPIO_PIN_13

#endif
