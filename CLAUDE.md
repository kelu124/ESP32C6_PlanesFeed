# esp32_plane

Firmware for a Waveshare ESP32-C6-LCD-1.3 that joins the local Wi-Fi and shows the top 3 planes overhead on its LCD. Board details are in `docs/board.md`.

## Working rules
- At session start, review `.claude/TODO.md` with the user. A hook prints it automatically.
- Every commit is documented: a commit message that explains what changed and why, plus a matching entry in `.claude/DONE.md` in the same commit.
- When a task is finished, move it from `.claude/TODO.md` to `.claude/DONE.md`.
- Never commit Wi-Fi credentials.
