# ESP32-C3 AT Flasher Release

This directory is the output target for `tools/flasher/build_exe.ps1`.

Expected release file:

```text
ESP32C3_AT_Flasher.exe
```

The packaged EXE embeds the `firmware/` directory and does not require Python on the target Windows PC. Flash logs are written to `release/logs/`.
