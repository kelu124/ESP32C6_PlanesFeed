# Make plane requests reliable on the board

Date: 2026-11-08

## What changed
- **Firmware, `feed.cpp`:** the plane and route replies are read in full before being parsed. The connection and TLS handshake now have explicit 8-second limits.
- **Firmware, `esp32_plane.ino`:** Wi-Fi power saving is off.
- **Docs:** `docs/design.md` records the power-saving choice and the 2.4 GHz requirement, and corrects the note on serial tools.
- **Tasks:** Wi-Fi setup and live data moved to `DONE.md`.

## Why
The first test on real Wi-Fi showed about 40% of plane requests failing, in three different ways:
- **Replies that wouldn't parse (`IncompleteInput`):** on a TLS client, `read()` returns -1 when no data is ready yet, and `NetworkClient::readBytes()` treats that as an error. ArduinoJson then saw any pause between packets as the end of the reply.
- **Requests hanging for over 45 seconds:** the library's TLS handshake limit defaults to 120 seconds.
- **Connections failing after about 5 seconds:** Wi-Fi power saving slowed traffic enough to hit the library's default 5-second limit. Turning power saving off removed these failures. The board runs from USB, so the extra power use doesn't matter.

## Testing
On the board, with live Wi-Fi and a build that had temporary logging:
- Before the fixes: 10 of 17 plane requests succeeded in 5 minutes.
- With the read and timeout fixes: 13 of 18 succeeded, with no parse failures.
- With power saving also off: 26 of 26 succeeded in 5 minutes, and the slowest request took 1.5 seconds. No restarts.

On the board, with the final build and the logging removed: every `status` check over 2 minutes showed a successful fetch with current planes, and a real route appeared (Algiers > Paris). `make test` passes.

Not tested:
- What the five LCD screens look like, and the button. These need someone at the board.
- Running for longer than 5 minutes at a time.
