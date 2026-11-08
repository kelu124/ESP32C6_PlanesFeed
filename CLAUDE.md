# esp32_plane

Firmware for a Waveshare ESP32-C6-LCD-1.3 that joins the local Wi-Fi and shows the top 3 planes overhead on its LCD. Board details are in `docs/board.md`.

## Working rules
- At session start, review `.claude/TODO.md` with the user. A hook prints it automatically.
- Every commit is documented in `.claude/commits/`, in the same commit. Follow the `commit-docs` skill in `.claude/skills/`.
- When a task is finished, move it from `.claude/TODO.md` to `.claude/DONE.md`.
- Never commit Wi-Fi credentials.
