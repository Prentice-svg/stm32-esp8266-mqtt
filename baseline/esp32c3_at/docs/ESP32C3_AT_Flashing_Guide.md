# ESP32-C3 Super Mini ESP-AT 刷机指南

## 固件版本

本基准锁定官方固件：

```text
ESP32-C3-MINI-1-AT-V4.1.1.0
```

固件文件位于：

```text
baseline/esp32c3_at/firmware/
```

`manifest.json` 记录官方来源、SHA256、AT UART 引脚和当前 STM32 基准接线。

## 一键刷机

发布版路径：

```text
baseline/esp32c3_at/release/ESP32C3_AT_Flasher.exe
```

双击后按提示选择 COM 口即可。程序默认流程：

1. 枚举串口。
2. 验证目标芯片为 ESP32-C3。
3. 擦除 Flash。
4. 根据官方 `flasher_args.json` 写入 bootloader、partition table、ota data、AT app、customize 和 mfg nvs。
5. 重启模块。
6. 发送 `AT` 和 `AT+GMR` 验证。
7. 保存日志到 `release/logs/`。

默认刷写波特率为 `460800`，失败后自动尝试 `115200`。

## 源码模式

如果没有 EXE，可以在安装 Python 后运行：

```powershell
python -m pip install -r baseline\esp32c3_at\tools\flasher\requirements.txt
python baseline\esp32c3_at\tools\flasher\esp32c3_at_flasher.py --check
python baseline\esp32c3_at\tools\flasher\esp32c3_at_flasher.py --port COM5
```

如果刷机口和 AT 命令口不是同一个串口，使用：

```powershell
python baseline\esp32c3_at\tools\flasher\esp32c3_at_flasher.py --port COM5 --at-port COM6
```

## 打包 EXE

在有 Python 的 Windows 电脑上运行：

```powershell
powershell -ExecutionPolicy Bypass -File baseline\esp32c3_at\tools\flasher\build_exe.ps1
```

脚本会建立本地构建虚拟环境，安装 PyInstaller、esptool、pyserial，并输出：

```text
baseline/esp32c3_at/release/ESP32C3_AT_Flasher.exe
```

## 下载模式提示

如果程序提示无法连接 ESP32-C3：

1. 按住 ESP32-C3 Super Mini 的 BOOT。
2. 点按 RST 或重新插 USB。
3. 松开 BOOT。
4. 重新执行刷机。

不同批次 Super Mini 的 USB、BOOT、RST 设计可能不同。刷机成功但 `AT` 验证失败时，优先确认 AT 命令口是否接在 GPIO7/GPIO6，而不是 USB 下载口。
