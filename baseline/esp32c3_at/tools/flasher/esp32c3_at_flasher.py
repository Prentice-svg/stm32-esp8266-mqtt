#!/usr/bin/env python3
"""Windows-oriented ESP32-C3 ESP-AT flasher for the STM32F103 baseline."""

from __future__ import annotations

import argparse
import contextlib
import json
import os
from pathlib import Path
import sys
import time
import traceback
from typing import Iterable


DEFAULT_BAUDS = [460800, 115200]
AT_BAUD = 115200


class Tee:
    def __init__(self, *streams):
        self.streams = streams

    def write(self, data: str) -> int:
        for stream in self.streams:
            stream.write(data)
            stream.flush()
        return len(data)

    def flush(self) -> None:
        for stream in self.streams:
            stream.flush()


def is_frozen() -> bool:
    return bool(getattr(sys, "frozen", False))


def app_root() -> Path:
    if is_frozen():
        return Path(sys.executable).resolve().parent
    return Path(__file__).resolve().parents[2]


def resource_root() -> Path:
    if is_frozen() and hasattr(sys, "_MEIPASS"):
        return Path(sys._MEIPASS)  # type: ignore[attr-defined]
    return app_root()


def firmware_root_from_args(value: str | None) -> Path:
    if value:
        return Path(value).expanduser().resolve()

    packaged = resource_root() / "firmware"
    if packaged.exists():
        return packaged

    return app_root() / "firmware"


def load_manifest(firmware_root: Path) -> dict:
    manifest_path = firmware_root / "manifest.json"
    if not manifest_path.exists():
        raise FileNotFoundError(f"Firmware manifest not found: {manifest_path}")
    return json.loads(manifest_path.read_text(encoding="utf-8"))


def find_flasher_args(firmware_root: Path, manifest: dict) -> tuple[Path, dict]:
    preferred_dir = firmware_root / manifest.get("firmware_payload_dir", "")
    candidates: list[Path] = []
    if preferred_dir.exists():
        candidates.append(preferred_dir / manifest.get("flasher_args", "flasher_args.json"))
    candidates.extend(firmware_root.rglob("flasher_args.json"))

    for path in candidates:
        if path.exists():
            return path.parent, json.loads(path.read_text(encoding="utf-8"))

    raise FileNotFoundError(f"flasher_args.json not found under {firmware_root}")


def require_python_packages() -> bool:
    ok = True
    try:
        import esptool  # noqa: F401
    except Exception as exc:
        ok = False
        print(f"[MISSING] esptool: {exc}")

    try:
        import serial  # noqa: F401
        import serial.tools.list_ports  # noqa: F401
    except Exception as exc:
        ok = False
        print(f"[MISSING] pyserial: {exc}")

    if not ok:
        print()
        print("Install dependencies in source mode:")
        print("  python -m pip install -r baseline\\esp32c3_at\\tools\\flasher\\requirements.txt")
        print("The packaged EXE build bundles these dependencies and does not require Python on the target PC.")

    return ok


def list_ports() -> list[object]:
    import serial.tools.list_ports

    return list(serial.tools.list_ports.comports())


def choose_port(given: str | None) -> str:
    if given:
        return given

    ports = list_ports()
    if not ports:
        raise RuntimeError("No serial ports found. Connect the ESP32-C3 or USB-UART adapter first.")

    print("Available serial ports:")
    for index, port in enumerate(ports, 1):
        desc = getattr(port, "description", "")
        hwid = getattr(port, "hwid", "")
        print(f"  {index}. {port.device}  {desc}  {hwid}")

    if len(ports) == 1:
        print(f"Using only detected port: {ports[0].device}")
        return str(ports[0].device)

    while True:
        value = input("Select port number or type COM name: ").strip()
        if not value:
            continue
        if value.isdigit():
            index = int(value)
            if 1 <= index <= len(ports):
                return str(ports[index - 1].device)
        return value.upper()


def normalize_bauds(value: str | None) -> list[int]:
    if not value:
        return DEFAULT_BAUDS
    bauds = []
    for part in value.split(","):
        baud = int(part.strip())
        if baud not in bauds:
            bauds.append(baud)
    if 115200 not in bauds:
        bauds.append(115200)
    return bauds


def run_esptool(args: list[str]) -> None:
    print()
    print("esptool " + " ".join(args))
    try:
        import esptool
    except Exception as exc:
        raise RuntimeError(f"esptool is not available: {exc}") from exc

    if hasattr(esptool, "main"):
        try:
            result = esptool.main(args)  # type: ignore[attr-defined]
        except TypeError:
            old_argv = sys.argv[:]
            try:
                sys.argv = ["esptool"] + args
                result = esptool.main()  # type: ignore[attr-defined]
            finally:
                sys.argv = old_argv
        except SystemExit as exc:
            code = exc.code if isinstance(exc.code, int) else 1
            if code:
                raise RuntimeError(f"esptool exited with code {code}")
            return

        if isinstance(result, int) and result != 0:
            raise RuntimeError(f"esptool returned {result}")
        return

    raise RuntimeError("Installed esptool package does not expose a supported main() entry point.")


def build_write_flash_args(payload_dir: Path, flasher_args: dict, port: str, baud: int) -> list[str]:
    extra = flasher_args.get("extra_esptool_args", {})
    chip = extra.get("chip", "esp32c3")
    before = extra.get("before", "default_reset")
    after = extra.get("after", "hard_reset")

    args = [
        "--chip",
        chip,
        "--port",
        port,
        "--baud",
        str(baud),
        "--before",
        before,
        "--after",
        after,
        "write_flash",
    ]
    args.extend(flasher_args.get("write_flash_args", []))

    flash_files = flasher_args.get("flash_files", {})
    if not flash_files:
        raise RuntimeError("flasher_args.json does not contain flash_files")

    for offset, rel_path in sorted(flash_files.items(), key=lambda item: int(item[0], 16)):
        bin_path = payload_dir / rel_path
        if not bin_path.exists():
            raise FileNotFoundError(f"Firmware binary not found: {bin_path}")
        args.extend([offset, str(bin_path)])

    return args


def chip_id_args(flasher_args: dict, port: str, baud: int) -> list[str]:
    extra = flasher_args.get("extra_esptool_args", {})
    return [
        "--chip",
        extra.get("chip", "esp32c3"),
        "--port",
        port,
        "--baud",
        str(baud),
        "--before",
        extra.get("before", "default_reset"),
        "--after",
        "no_reset",
        "chip_id",
    ]


def erase_args(flasher_args: dict, port: str, baud: int) -> list[str]:
    extra = flasher_args.get("extra_esptool_args", {})
    return [
        "--chip",
        extra.get("chip", "esp32c3"),
        "--port",
        port,
        "--baud",
        str(baud),
        "--before",
        extra.get("before", "default_reset"),
        "--after",
        "no_reset",
        "erase_flash",
    ]


def verify_at(port: str, retries: int = 8) -> str:
    import serial

    last_response = ""
    for attempt in range(1, retries + 1):
        print(f"AT check on {port}, attempt {attempt}/{retries}")
        try:
            with serial.Serial(port, AT_BAUD, timeout=1.0, write_timeout=1.0) as ser:
                ser.reset_input_buffer()
                ser.write(b"AT\r\n")
                ser.flush()
                time.sleep(0.35)
                response = ser.read(512).decode(errors="replace")
                last_response = response
                if "OK" in response:
                    ser.write(b"AT+GMR\r\n")
                    ser.flush()
                    time.sleep(0.5)
                    gmr = ser.read(2048).decode(errors="replace")
                    print("AT verification OK.")
                    print(gmr.strip() or response.strip())
                    return gmr or response
        except Exception as exc:
            last_response = str(exc)
        time.sleep(0.7)

    raise RuntimeError(
        "AT verification failed. If you flashed through USB/JTAG, connect a USB-UART adapter "
        "to ESP32-C3 GPIO7(TX)/GPIO6(RX) and rerun with --at-port COMx. Last response: "
        + last_response
    )


def check_firmware_files(payload_dir: Path, flasher_args: dict) -> None:
    print(f"Firmware payload: {payload_dir}")
    print("Flash files:")
    for offset, rel_path in sorted(flasher_args.get("flash_files", {}).items(), key=lambda item: int(item[0], 16)):
        path = payload_dir / rel_path
        status = "OK" if path.exists() else "MISSING"
        size = path.stat().st_size if path.exists() else 0
        print(f"  {offset:>8}  {rel_path:<45} {status} {size} bytes")
        if not path.exists():
            raise FileNotFoundError(path)


def run_check(args: argparse.Namespace) -> int:
    print("ESP32-C3 AT flasher environment check")
    print(f"Mode: {'packaged EXE' if is_frozen() else 'source script'}")
    print(f"Python: {sys.version.split()[0]} ({sys.executable})")
    packages_ok = require_python_packages()

    firmware_root = firmware_root_from_args(args.firmware_root)
    manifest = load_manifest(firmware_root)
    payload_dir, flasher_args = find_flasher_args(firmware_root, manifest)
    print(f"Firmware: {manifest.get('esp_at_package')}")
    print(f"Source: {manifest.get('source_url')}")
    check_firmware_files(payload_dir, flasher_args)

    if packages_ok:
        ports = list_ports()
        if ports:
            print("Serial ports:")
            for port in ports:
                print(f"  {port.device}  {port.description}")
        else:
            print("Serial ports: none detected")

    return 0 if packages_ok else 2


def flash_once(port: str, at_port: str, baud: int, payload_dir: Path, flasher_args: dict, erase: bool, verify: bool) -> None:
    print(f"Using flash port {port} at {baud} baud")
    run_esptool(chip_id_args(flasher_args, port, baud))
    if erase:
        run_esptool(erase_args(flasher_args, port, baud))
    run_esptool(build_write_flash_args(payload_dir, flasher_args, port, baud))
    if verify:
        verify_at(at_port)


def make_log_file() -> Path:
    log_dir = app_root() / "logs" if is_frozen() else app_root() / "release" / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    return log_dir / time.strftime("flash_%Y%m%d_%H%M%S.log")


def run_flash(args: argparse.Namespace) -> int:
    if not require_python_packages():
        return 2

    firmware_root = firmware_root_from_args(args.firmware_root)
    manifest = load_manifest(firmware_root)
    payload_dir, flasher_args = find_flasher_args(firmware_root, manifest)
    check_firmware_files(payload_dir, flasher_args)

    port = choose_port(args.port)
    at_port = args.at_port or port
    bauds = normalize_bauds(args.baud)

    errors: list[str] = []
    for baud in bauds:
        try:
            flash_once(port, at_port, baud, payload_dir, flasher_args, not args.no_erase, not args.no_verify_at)
            print()
            print("FLASH SUCCESS")
            return 0
        except Exception as exc:
            message = f"{baud} baud failed: {exc}"
            errors.append(message)
            print()
            print(message)
            if len(bauds) > 1:
                print("Trying next baud rate...")

    print()
    print("FLASH FAILED")
    for error in errors:
        print("  " + error)
    return 1


def parse_args(argv: Iterable[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Flash ESP-AT to ESP32-C3 Super Mini for STM32F103 baseline use.")
    parser.add_argument("--check", action="store_true", help="Check Python packages, firmware files, and serial ports.")
    parser.add_argument("--port", help="ESP32-C3 flashing serial port, for example COM5.")
    parser.add_argument("--at-port", help="AT command serial port after flashing. Defaults to --port.")
    parser.add_argument("--baud", help="Comma-separated flashing baud rates. Default: 460800,115200.")
    parser.add_argument("--firmware-root", help="Override firmware directory that contains manifest.json.")
    parser.add_argument("--no-erase", action="store_true", help="Skip erase_flash before write_flash.")
    parser.add_argument("--no-verify-at", action="store_true", help="Skip AT/AT+GMR verification after flashing.")
    parser.add_argument("--keep-open", action="store_true", help="Wait for Enter before closing the window.")
    return parser.parse_args(list(argv))


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    log_file = make_log_file()

    with log_file.open("w", encoding="utf-8") as log:
        tee_out = Tee(sys.stdout, log)
        tee_err = Tee(sys.stderr, log)
        with contextlib.redirect_stdout(tee_out), contextlib.redirect_stderr(tee_err):
            print(f"Log file: {log_file}")
            try:
                if args.check:
                    return run_check(args)
                return run_flash(args)
            except KeyboardInterrupt:
                print("Interrupted by user.")
                return 130
            except Exception:
                traceback.print_exc()
                return 1
            finally:
                print(f"Log saved: {log_file}")
                if args.keep_open or (is_frozen() and os.environ.get("ESP32C3_FLASHER_NO_PAUSE") != "1"):
                    input("Press Enter to exit...")


if __name__ == "__main__":
    raise SystemExit(main())
