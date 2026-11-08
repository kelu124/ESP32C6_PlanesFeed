# Plane tracker design

## Goal
A Waveshare ESP32-C6-LCD-1.3 on the local Wi-Fi shows the three planes closest to overhead.

## Agreed behaviour
- **Top 3:** the planes with the smallest horizontal distance from the point directly above the saved location. Altitude does not affect the order.
- **Button:** each press of BOOT moves through five screens: planes as rows, closest plane first, radar, Wi-Fi, location.
- **Closest-plane screen:** also shows origin and destination, or "Route unknown".
- **CLI over USB serial:** sets the Wi-Fi and the location. `dfu` restarts into the chip's serial download mode, because the ESP32-C6 has no true USB DFU.
- **Persistence:** Wi-Fi details and location are stored in the NVS data partition on the onboard flash, separate from the firmware, so firmware updates keep them.
- **Plane data:** adsb.fi, under the rules in `data-sources.md`.

## Builder's calls
- Search radius of 20 km, refreshed every 10 seconds.
- Planes on the ground are skipped.
- The last-used screen is saved and restored at power-on.
- Planes screens explain what is missing when Wi-Fi or location isn't set, and say when no planes are nearby. Data older than 30 seconds shows its age.
- The Wi-Fi password is stored unencrypted in flash, so anyone with physical access to the board could extract it. It is never shown on the LCD or by `status`.
- HTTPS certificates are not verified. The data is public and no credentials are sent, so the worst case is wrong planes on the screen.
- The partition scheme is `huge_app` (3 MB firmware, no over-the-air updates).
- The Wi-Fi network must be on 2.4 GHz. The ESP32-C6 has no 5 GHz radio.
- Wi-Fi power saving is off. With it on, about a quarter of HTTPS connections timed out. The board runs from USB, so the extra power use doesn't matter.

## Structure
| File in `firmware/esp32_plane/` | Purpose |
|---|---|
| `logic.*` | Distance, sorting, text and CLI parsing, with host tests in `test/` |
| `settings.*` | Saved settings in NVS |
| `feed.*` | Background task that fetches planes and routes |
| `ui.*` | The five screens |
| `esp32_plane.ino` | Startup, CLI and button |

## Commands
| Command | Purpose |
|---|---|
| `make test` | Run the logic tests on this computer |
| `make flash` | Build and flash the firmware |
| `make setup` | Send `.env` values to the board's CLI |
| `make leave-dfu` | Leave download mode and restart the firmware |

## Leaving download mode
The `dfu` command sets the chip's `LP_AON_FORCE_DOWNLOAD_BOOT` flag, which survives a normal reset. The board keeps returning to download mode until the flag is cleared or the power is removed. `make flash` and `make leave-dfu` clear it. Tested on the board on 2026-11-08.

Serial tools should open the port with DTR and RTS off. Asserting them on open can reset this chip into download mode, as a test script did during development. Even with both off, opening the port restarts the firmware, so Wi-Fi reconnects each time.

Personal values go in `.env`, which is ignored by git. `.env.example` lists the keys.
