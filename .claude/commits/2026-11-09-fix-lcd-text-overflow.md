# Stop LCD values covering labels and routes overflowing

Date: 2026-11-09

## What changed
- **Key and value rows (`ui.cpp`):** a value's padding is now limited to the space beside its label, instead of a fixed 14 characters.
- **Route line (`feed.cpp`):** `ROUTE_TEXT_CHARS` is now 18, down from 20. Longer city pairs fall back to airport codes.
- **Images:** `docs/images/location.png` was rendered again to show the corrected label.

## Why
The fixed padding was drawn over the end of the "Search radius" label on the Location screen. The padding is still needed, because the Wi-Fi screen redraws without clearing and has to erase the old value.

The route line is 224 px wide and fits 18 characters at 12 px each, so a 19 or 20 character route ran past the margin and a 20 character route was cut off at the screen edge.

## Testing
- Rendered the real `ui.cpp` on the computer and counted lit pixels. Before the fix, the hidden part of the label had 0 and a 20 character route had 8 past the right margin. After the fix, the label has 45 and the route has 0.
- A route of exactly 18 characters stays within the margin. Renders of the other four screens are byte-identical to before.
- `make test` passed and `make firmware` built without errors.
- The new firmware was not flashed or checked on the board.
