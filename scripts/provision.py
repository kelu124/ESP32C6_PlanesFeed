#!/usr/bin/env python3
"""Send the Wi-Fi details and location from .env to the board's USB CLI.

The board saves them in flash, so this only needs to run once.
"""
import argparse
import pathlib
import sys
import time

import serial

ENV_PATH = pathlib.Path(__file__).resolve().parent.parent / ".env"
COMMAND_TIMEOUT_S = 5.0


def read_env(path):
    values = {}
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        values[key.strip()] = value.strip().strip('"')
    return values


def quote(value):
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'"{escaped}"'


def send(port, command, label):
    port.reset_input_buffer()
    port.write(command.encode("utf-8") + b"\n")
    deadline = time.monotonic() + COMMAND_TIMEOUT_S
    while time.monotonic() < deadline:
        reply = port.readline().decode("utf-8", "replace").strip()
        if reply.startswith("ok:"):
            print(f"{label}: {reply}")
            return True
        if reply.startswith("error:"):
            print(f"{label}: {reply}", file=sys.stderr)
            return False
    print(f"{label}: no reply from the board", file=sys.stderr)
    return False


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", default="/dev/ttyACM0")
    args = parser.parse_args()

    if not ENV_PATH.exists():
        sys.exit(f"{ENV_PATH} not found; copy .env.example and fill it in")
    env = read_env(ENV_PATH)

    # Leave DTR and RTS off: asserting them on open can reset this chip into download mode.
    port = serial.Serial()
    port.port = args.port
    port.baudrate = 115200
    port.timeout = 0.5
    port.dtr = False
    port.rts = False
    with port:
        time.sleep(0.5)
        ok = True
        if env.get("LATITUDE") and env.get("LONGITUDE"):
            ok &= send(port, f"location {env['LATITUDE']} {env['LONGITUDE']}", "location")
        else:
            print("location: skipped, LATITUDE or LONGITUDE is empty")
        if env.get("WIFI_SSID"):
            command = f"wifi {quote(env['WIFI_SSID'])} {quote(env.get('WIFI_PASSWORD', ''))}"
            ok &= send(port, command, "wifi")
        else:
            print("wifi: skipped, WIFI_SSID is empty")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
