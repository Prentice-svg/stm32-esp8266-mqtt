$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$uvprojx = Join-Path $projectRoot 'iot_terminal\Project\IoT_Terminal.uvprojx'
$hex = Join-Path $projectRoot 'iot_terminal\Project\Objects\STM32F103_Project.hex'
$buildLog = Join-Path $projectRoot 'iot_terminal\Project\build_daplink_openocd.log'
$keil = 'D:\Keil_v5\UV4\UV4.exe'
$openocd = 'C:\Users\13158\Downloads\xpack-openocd-0.12.0-6\bin\openocd.exe'

if (!(Test-Path $keil)) {
  throw "Keil UV4 not found: $keil"
}
if (!(Test-Path $openocd)) {
  throw "OpenOCD not found: $openocd"
}

& $keil -b $uvprojx -j0 -o $buildLog
if (!(Test-Path $buildLog) -or !((Get-Content $buildLog -Raw) -match '0 Error\(s\)')) {
  if (Test-Path $buildLog) {
    Get-Content $buildLog
  }
  throw "Keil build failed."
}

if (!(Test-Path $hex)) {
  throw "HEX not found: $hex"
}

$hexOpenOcd = $hex -replace '\\','/'

& $openocd `
  -f interface/cmsis-dap.cfg `
  -f target/stm32f1x.cfg `
  -c 'adapter speed 1000' `
  -c 'transport select swd' `
  -c 'init' `
  -c 'reset halt' `
  -c "flash write_image erase $hexOpenOcd" `
  -c "verify_image $hexOpenOcd" `
  -c 'reset run' `
  -c 'shutdown'

if ($LASTEXITCODE -ne 0) {
  throw "OpenOCD flash failed."
}

Write-Host 'DAPLink OpenOCD flash OK.'
