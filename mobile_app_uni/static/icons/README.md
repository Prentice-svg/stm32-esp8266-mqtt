# Icon Assets

This directory contains separated dashboard icons extracted from the source
images in `E:\Dev\stm32-esp-mqtt\repo\demo`.

## Files

- `*_basic.png`: icons from the first 12-item source image.
- `*_sensor_a.png`: sensor icons from source image A.
- `*_device_b.png`: device, switch, status, and actuator icons from source image B.
- `*_control_c.png`: control and electrical icons from source image C.
- `*_system_d.png`: system and environment icons from source image D.
- `icons.json`: machine-readable icon index with English labels and uni-app paths.
- `_preview.png`: visual preview of the extracted icon set.

## Usage

Use the `file` value from `icons.json` directly in uni-app image components.

```vue
<image src="/static/icons/temperature_sensor_sensor_a.png" mode="aspectFit" />
```

When adding a new sensor, switch, or feature, pick the closest English-named
icon from `icons.json` and reference its `/static/icons/...` path.
