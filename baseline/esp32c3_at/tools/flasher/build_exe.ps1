param(
    [string]$Python = $env:PYTHON
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BaselineRoot = Resolve-Path (Join-Path $ScriptDir "..\..")
$ReleaseDir = Join-Path $BaselineRoot "release"
$FirmwareDir = Join-Path $BaselineRoot "firmware"
$VenvDir = Join-Path $ScriptDir ".venv-build"
$BuildDir = Join-Path $ScriptDir "build"
$Entry = Join-Path $ScriptDir "esp32c3_at_flasher.py"
$Req = Join-Path $ScriptDir "requirements.txt"

function Find-Python {
    if ($Python) {
        $resolved = Resolve-Path $Python -ErrorAction Stop
        return [PSCustomObject]@{ Exe = $resolved.Path; Args = @() }
    }

    $candidates = @("python", "py")
    foreach ($cmd in $candidates) {
        $found = Get-Command $cmd -ErrorAction SilentlyContinue
        if ($found) {
            if ($cmd -eq "py") {
                return [PSCustomObject]@{ Exe = $found.Source; Args = @("-3") }
            }
            return [PSCustomObject]@{ Exe = $found.Source; Args = @() }
        }
    }
    throw "Python was not found. Install Python 3.10+ or build on a machine that has Python."
}

New-Item -ItemType Directory -Force -Path $ReleaseDir | Out-Null

$pythonInfo = Find-Python
$pythonExe = $pythonInfo.Exe
$pythonArgs = @($pythonInfo.Args)
Write-Host "Using Python command: $pythonExe $($pythonArgs -join ' ')"

if (-not (Test-Path $VenvDir)) {
    & $pythonExe @pythonArgs -m venv $VenvDir
}

$VenvPython = Join-Path $VenvDir "Scripts\python.exe"
& $VenvPython -m pip install --upgrade pip
& $VenvPython -m pip install -r $Req

& $VenvPython -m PyInstaller `
    --clean `
    --onefile `
    --name ESP32C3_AT_Flasher `
    --distpath $ReleaseDir `
    --workpath $BuildDir `
    --specpath $ScriptDir `
    --collect-all esptool `
    --add-data "$FirmwareDir;firmware" `
    $Entry

Write-Host ""
Write-Host "Built: $(Join-Path $ReleaseDir 'ESP32C3_AT_Flasher.exe')"
Write-Host "Run source check:"
Write-Host "  $VenvPython $Entry --check"
