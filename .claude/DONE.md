# DONE

Newest first. Finished tasks live here; each commit is documented in `commits/`.

## 2026-11-08: Live data on the board
- Saved the Wi-Fi details with `make setup`; the board joins the 2.4 GHz network.
- Confirmed planes and routes arrive over the CLI `status` command.
- Fixed unreliable requests: 26 of 26 succeeded in a 5-minute test.

## 2026-11-08: Plane tracker firmware
- Agreed the design in `docs/design.md`, using adsb.fi for planes and adsbdb for routes.
- Built the firmware: five screens, button cycling, USB CLI, saved Wi-Fi and location, `dfu` command.
- Added `make test`, `make flash`, `make setup` and `make leave-dfu`.
- Added the `commit-docs` skill and documented the adsb.fi usage rules.

## 2026-11-08: Project setup
- Created `.claude/` with hooks, skills, `TODO.md` and `DONE.md`.
- Added a SessionStart hook that shows `TODO.md` at the start of each session.
- Initialised the git repository.
- Saved the board reference and schematic in `docs/`.
