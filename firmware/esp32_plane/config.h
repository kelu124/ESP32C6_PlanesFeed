// Board pins and application constants. Pins are from docs/board.md.
#pragma once

static const int PIN_SPI_MISO = 5;
static const int PIN_SPI_MOSI = 6;
static const int PIN_SPI_SCLK = 7;
static const int PIN_LCD_CS = 14;
static const int PIN_LCD_DC = 15;
static const int PIN_LCD_RST = 21;
static const int PIN_LCD_BACKLIGHT = 22;
static const int PIN_BUTTON = 9;  // the BOOT button

static const int LCD_SIZE = 240;
static const uint32_t LCD_SPI_HZ = 40000000;
static const int BACKLIGHT_DUTY = 800;  // out of 1023

static const float SEARCH_RADIUS_KM = 20.0f;
// The feed takes whole nautical miles. 11 nm is 20.4 km; results are then cut to 20 km.
static const int SEARCH_RADIUS_NM = 11;

// adsb.fi allows 1 request per second; see docs/data-sources.md.
static const uint32_t FEED_INTERVAL_MS = 10000;
static const uint32_t FEED_BACKOFF_MS = 60000;  // after HTTP 429
static const uint32_t STALE_AFTER_MS = 30000;

enum Screen {
  SCREEN_ROWS,
  SCREEN_CLOSEST,
  SCREEN_RADAR,
  SCREEN_WIFI,
  SCREEN_LOCATION,
  SCREEN_COUNT
};
