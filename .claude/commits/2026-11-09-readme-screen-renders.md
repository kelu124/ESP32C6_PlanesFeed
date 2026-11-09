# Show renders of the five screens in the README

Date: 2026-11-09

## What changed
- **Images:** added one PNG per screen in `docs/images/`, at 480 x 480 (twice the LCD size).
- **README:** the screens table now shows each render, with a note that they use sample data.

## Why
A picture of each screen explains the tool faster than the table alone.

The renders come from the real `ui.cpp`, compiled on the computer against a stand-in display that uses the same 5 x 7 font as Arduino_GFX. The data is made up: a public landmark as the location, a placeholder network and invented traffic, so no `.env` values appear. The stand-in was a throwaway and is not in the repository.

## Testing
All five images were checked by eye. They were not compared with the physical LCD, so curves and arrows can differ by a pixel or so from the real display. No firmware code changed, so the firmware was not rebuilt or flashed.

The Location render shows an existing bug: the "Search radius" label is partly hidden by the padded value next to it. It is not fixed here.
