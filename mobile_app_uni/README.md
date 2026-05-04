# Mobile App

This folder is a cleaned `uni-app` version of the mobile dashboard from the
original repository.

## What it does

- Polls OneNET device properties every 2 seconds
- Displays:
  - humidity
  - temperature
  - pressure
  - light
  - LED state
- Sends LED on/off updates through the OneNET property API

## Main files

- [pages/index/index.vue](/E:/Dev/stm32-esp-mqtt/repo/mobile_app_uni/pages/index/index.vue)
- [key.js](/E:/Dev/stm32-esp-mqtt/repo/mobile_app_uni/key.js)
- [manifest.json](/E:/Dev/stm32-esp-mqtt/repo/mobile_app_uni/manifest.json)
- [pages.json](/E:/Dev/stm32-esp-mqtt/repo/mobile_app_uni/pages.json)

## How to open

Open this folder in `HBuilderX` as a `uni-app` project:

- [mobile_app_uni](/E:/Dev/stm32-esp-mqtt/repo/mobile_app_uni)

Then use:

1. `Run -> Run to Browser`
2. `Run -> Run to Android App Base`
3. or build an Android package from HBuilderX

## Notes

- The current token is generated in the app from the product `access_key`.
- Product and device defaults are hardcoded for the current demo device:
- `productId = 45Z51DlJn3`
- `deviceName = stm32-esp32c3`
- If you move to a new OneNET product or device, update those values in
  `pages/index/index.vue`.
- Current OneNET identifiers are `Humi`, `Temp`, `Light`, `Press`, and `led`.
