# Add plane tracker firmware and project tooling

Date: 2026-11-08

## What changed
- **Firmware** (`firmware/esp32_plane/`): five LCD screens cycled by the button, a USB serial CLI, Wi-Fi and location saved in the NVS partition, and a background task that fetches planes and routes.
- **Tooling:** `Makefile` targets for tests, flashing, setup and leaving download mode. `scripts/provision.py` sends `.env` values to the CLI. `test/test_logic.cpp` covers the board-independent logic.
- **Docs:** `docs/design.md` and `docs/data-sources.md`, which includes the adsb.fi usage rules.
- **Project rules:** startup hook for `TODO.md`, the `commit-docs` skill, and this `.claude/commits/` folder.

## Why
The board should show the three planes closest to overhead on its own, set up once over USB. Settings are in a data partition so firmware updates keep them. Personal values stay in `.env`, which git ignores.

adsb.fi was chosen because it answered fastest in tests and includes full aircraft names. It allows 1 request per second, and the firmware makes one every 10 seconds.

The ESP32-C6 has no true USB DFU, so `dfu` restarts into the chip's serial download mode. Its flag survives a normal reset, so `make flash` and `make leave-dfu` clear it.

## Testing
On this computer:
- `make test`: all logic tests pass.
- The adsb.fi and adsbdb requests return the expected data when run with `curl`.

On the board:
- The firmware compiles and flashes, and the CLI rejects bad input.
- The location survives a reboot and a reflash.
- `dfu` enters download mode, and three out of three `dfu` and `make leave-dfu` cycles returned to the firmware. `dfu` followed by `make flash` also works.

Not tested:
- Wi-Fi and live plane data on the board, because `.env` has no Wi-Fi details yet.
- The LCD screens and the button, which need someone at the board.
