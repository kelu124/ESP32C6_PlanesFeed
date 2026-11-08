# Data sources

## Aircraft positions: adsb.fi (primary)
- Endpoint: `https://opendata.adsb.fi/api/v3/lat/{lat}/lon/{lon}/dist/{nautical miles}`
- Project: https://github.com/adsbfi/opendata
- No API key is needed.

### Usage rules (from the adsb.fi open data README, read 2026-11-08)
- **Personal, non-commercial use only.** The data and service may not be licensed, sold, rented or leased.
- Public endpoints are **limited to 1 request per second**. The feeder endpoint is limited to 1 request every 30 seconds.
- Requests that return 400, 401, 403, 404 or 429 **count toward the limit**, and users should make reasonable efforts to avoid invalid requests.
- Commercial use or higher request rates require contacting adsb.fi.

### How this firmware complies
- It makes one request every 10 seconds, with `FEED_INTERVAL_MS` in `firmware/esp32_plane/config.h`.
- It sends no request until both Wi-Fi and a valid location are set, and the CLI rejects out-of-range coordinates.
- After HTTP 429 it waits 60 seconds before trying again (`FEED_BACKOFF_MS`).
- This project is for personal use. Using it commercially would need permission from adsb.fi.

## Flight routes: adsbdb
- Endpoint: `https://api.adsbdb.com/v0/callsign/{callsign}`
- Used only for the closest plane, and only when that plane changes.
- Routes come from a database lookup by flight number, so they can occasionally be wrong. Private flights often have no entry, and the screen then shows "Route unknown".
- The adsbdb README says the flight route data "may not be copied, published, or incorporated into other databases without the explicit permission of David J Taylor, Edinburgh." This firmware only shows the current route on the LCD and stores none of it.
- No request-rate limit was found in the adsbdb README or on its home page.

## Tested but not used
- **adsb.lol** (`https://api.adsb.lol/v2/point/{lat}/{lon}/{nautical miles}`) returned the same data format on 2026-11-08, so it is a possible fallback.
- **airplanes.live** returned HTTP 403 on 2026-11-08 and asks projects to email them for access.
