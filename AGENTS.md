# AGENTS.md

Scope: this entire `stm32-esp-mqtt` project tree.

## Project Entry Points

- Project root: `E:\Dev\stm32-esp-mqtt`
- Git repo: `E:\Dev\stm32-esp-mqtt\repo`
- STM32 firmware: `E:\Dev\stm32-esp-mqtt\repo\iot_terminal`
- uni-app dashboard: `E:\Dev\stm32-esp-mqtt\repo\mobile_app_uni`
- Project memory: `E:\Dev\stm32-esp-mqtt\PROJECT_MEMORY.md`
- Detailed guide: `E:\Dev\stm32-esp-mqtt\repo\STM32_ESP32C3_OneNet_Development_Guide.md`

Before changing firmware or app code, read `PROJECT_MEMORY.md` and the detailed guide.

## Active Skill

For future Codex work, use the local skill:

```text
$stm32-esp32c3-onenet-terminal
```

## Current Validated Stack

- STM32F103 firmware talks to ESP32-C3 running ESP-AT over USART3.
- ESP32-C3 AT UART: GPIO7 TX -> STM32 PB11 RX, GPIO6 RX -> STM32 PB10 TX.
- I2C bus: PB8 SCL, PB9 SDA.
- I2C devices: AHT20 `0x38`, OLED `0x3C`, BMP280 `0x77`.
- OneNet Product ID: `45Z51DlJn3`.
- OneNet Device Name: `stm32-esp32c3`.

## OneNet Identifiers

Use only the current identifiers unless the platform model is changed:

- `Humi`
- `Temp`
- `Light`
- `Press`
- `led`

Do not use the older identifiers `EnvironmentHumidity`, `EnvironmentTemperature`, or `LightLux`.

## Firmware Guardrails

- Use `AT+MQTTPUBRAW` for full property posts. Do not switch back to long `AT+MQTTPUB`.
- Keep `configPRIO_BITS = 3`.
- Keep `NVIC_SetPriorityGrouping(0)` after `HAL_Init()`.
- Keep ESP USART3 and DMA1_Channel3 IRQ priorities FreeRTOS-safe; current working value is `10`.
- Protect OLED, AHT20, and BMP280 shared I2C access with `I2C1BusMutex`.
- Upload `Light` with one decimal place because OneNet step is `0.1`.

## App Guardrails

- App lives in `repo/mobile_app_uni` and is opened with HBuilderX.
- Pressure module uses `static/pressure.png` and reads `Press`.
- Display pressure in `hPa`, two decimals.
