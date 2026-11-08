#include "ui.h"

#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <math.h>

#include "config.h"
#include "feed.h"
#include "settings.h"

static const uint16_t BLACK_BG = 0x0000;
static const uint16_t WHITE_TEXT = 0xEF7E;
static const uint16_t DIM_TEXT = 0x8CD4;
static const uint16_t AMBER = 0xFD80;
static const uint16_t GREEN = 0x56B1;
static const uint16_t RED = 0xF8C3;
static const uint16_t LINE = 0x2987;

static const int MARGIN = 8;
static const int BODY_TOP = 24;
static const int CHAR_W = 6;  // the built-in font is 6 x 8 at text size 1
static const int CHAR_H = 8;

static Arduino_DataBus *bus =
    new Arduino_ESP32SPI(PIN_LCD_DC, PIN_LCD_CS, PIN_SPI_SCLK, PIN_SPI_MOSI, PIN_SPI_MISO);
static Arduino_GFX *gfx = new Arduino_ST7789(bus, PIN_LCD_RST, 0, true, LCD_SIZE, LCD_SIZE);

static int currentScreen = 0;
static bool needsFull = true;
static uint32_t drawnSequence = 0;
static uint32_t lastSecondMs = 0;

static void text(int x, int y, int size, uint16_t color, const char *value) {
  gfx->setTextSize(size);
  gfx->setTextColor(color, BLACK_BG);
  gfx->setCursor(x, y);
  gfx->print(value);
}

static void textRight(int rightX, int y, int size, uint16_t color, const char *value) {
  text(rightX - (int)strlen(value) * CHAR_W * size, y, size, color, value);
}

static void header(const char *title) {
  text(MARGIN, 7, 1, DIM_TEXT, title);
  for (int i = 0; i < SCREEN_COUNT; i++) {
    int x = LCD_SIZE - MARGIN - 3 - (SCREEN_COUNT - 1 - i) * 11;
    gfx->fillCircle(x, 10, 3, i == currentScreen ? AMBER : LINE);
  }
  gfx->drawFastHLine(MARGIN, 20, LCD_SIZE - 2 * MARGIN, LINE);
}

// Draws a small arrow pointing along heading (degrees clockwise from north).
static void arrow(int cx, int cy, int radius, int heading, uint16_t color) {
  float a = heading * DEG_TO_RAD;
  float dx = sin(a), dy = -cos(a);
  int tipX = cx + dx * radius, tipY = cy + dy * radius;
  int tailX = cx - dx * radius, tailY = cy - dy * radius;
  gfx->drawLine(tailX, tailY, tipX, tipY, color);
  float wing = radius * 0.7f;
  gfx->fillTriangle(tipX, tipY,
                    tipX - dx * wing - dy * wing * 0.6f, tipY - dy * wing + dx * wing * 0.6f,
                    tipX - dx * wing + dy * wing * 0.6f, tipY - dy * wing - dx * wing * 0.6f, color);
}

static void formatKm(char *out, size_t size, float km) {
  snprintf(out, size, "%d km", (int)lroundf(km));
}

// Shows why there are no planes. Returns true when a message was drawn.
static bool drawEmptyState(const Settings &settings, const FeedSnapshot &feed) {
  const char *line1 = NULL;
  const char *line2 = NULL;
  if (settings.ssid.isEmpty()) {
    line1 = "Wi-Fi not set";
    line2 = "wifi <name> <password>";
  } else if (!settings.hasLocation) {
    line1 = "Location not set";
    line2 = "location <lat> <lon>";
  } else if (WiFi.status() != WL_CONNECTED) {
    line1 = "Connecting to Wi-Fi";
  } else if (!feed.hasData) {
    line1 = "Waiting for data";
  } else if (feed.count == 0) {
    line1 = "No planes";
    line2 = "within 20 km";
  }
  if (!line1) return false;
  text((LCD_SIZE - (int)strlen(line1) * CHAR_W * 2) / 2, 100, 2, AMBER, line1);
  if (line2) text((LCD_SIZE - (int)strlen(line2) * CHAR_W) / 2, 130, 1, WHITE_TEXT, line2);
  return true;
}

static void drawAge(const FeedSnapshot &feed) {
  if (!feed.hasData) return;
  uint32_t age = millis() - feed.lastOkMs;
  char buf[16] = "";
  if (age >= STALE_AFTER_MS) snprintf(buf, sizeof(buf), "%lus old  ", (unsigned long)(age / 1000));
  else strcpy(buf, "          ");
  text(LCD_SIZE - MARGIN - 66 - CHAR_W * 10, 7, 1, RED, buf);
}

static void drawRows(const FeedSnapshot &feed) {
  for (size_t i = 0; i < feed.count; i++) {
    const Plane &p = feed.planes[i];
    int y = BODY_TOP + 8 + (int)i * 71;
    char km[12], line[40];
    formatKm(km, sizeof(km), p.distKm);
    text(MARGIN, y, 2, AMBER, p.flight);
    textRight(LCD_SIZE - MARGIN, y, 2, WHITE_TEXT, km);
    snprintf(line, sizeof(line), "%s  %d m  %d km/h", p.type, p.altM, p.speedKmh);
    text(MARGIN, y + 26, 1, DIM_TEXT, line);
    arrow(LCD_SIZE - MARGIN - 10, y + 29, 8, p.trackDeg, DIM_TEXT);
    if (i + 1 < TOP_PLANES) gfx->drawFastHLine(MARGIN, y + 54, LCD_SIZE - 2 * MARGIN, LINE);
  }
}

static void smallRow(int y, const Plane &p) {
  char km[12];
  formatKm(km, sizeof(km), p.distKm);
  text(MARGIN, y, 2, AMBER, p.flight);
  text(122, y + 4, 1, DIM_TEXT, p.type);
  textRight(LCD_SIZE - MARGIN, y, 2, WHITE_TEXT, km);
}

static void drawClosest(const FeedSnapshot &feed) {
  const Plane &p = feed.planes[0];
  char km[12], alt[16], dir[8];
  text(MARGIN, 30, 3, AMBER, p.flight);
  int descSize = (int)strlen(p.desc) * CHAR_W * 2 <= LCD_SIZE - 2 * MARGIN ? 2 : 1;
  text(MARGIN, 62, descSize, WHITE_TEXT, p.desc);
  text(MARGIN, 86, 2, GREEN, feed.route);

  formatKm(km, sizeof(km), p.distKm);
  snprintf(alt, sizeof(alt), "%d m", p.altM);
  snprintf(dir, sizeof(dir), "%s", compassName(p.trackDeg));
  text(MARGIN, 114, 2, WHITE_TEXT, km);
  text(92, 114, 2, WHITE_TEXT, alt);
  arrow(198, 121, 8, p.trackDeg, WHITE_TEXT);
  text(210, 114, 2, WHITE_TEXT, dir);

  gfx->drawFastHLine(MARGIN, 142, LCD_SIZE - 2 * MARGIN, LINE);
  if (feed.count > 1) smallRow(158, feed.planes[1]);
  if (feed.count > 2) smallRow(198, feed.planes[2]);
}

static void drawRadar(const FeedSnapshot &feed) {
  const int cx = LCD_SIZE / 2, cy = 88, outer = 62;
  gfx->drawCircle(cx, cy, outer, LINE);
  gfx->drawCircle(cx, cy, outer / 2, LINE);
  gfx->drawFastVLine(cx, cy - outer, 2 * outer, LINE);
  gfx->drawFastHLine(cx - outer, cy, 2 * outer, LINE);
  text(cx + 4, cy - outer, 1, DIM_TEXT, "N");
  gfx->fillCircle(cx, cy, 2, GREEN);

  for (size_t i = 0; i < feed.count; i++) {
    const Plane &p = feed.planes[i];
    float r = p.distKm / SEARCH_RADIUS_KM * outer;
    float a = p.bearingDeg * DEG_TO_RAD;
    int x = cx + sin(a) * r, y = cy - cos(a) * r;
    char label[2] = {(char)('1' + i), 0};
    gfx->fillCircle(x, y, 4, AMBER);
    text(x + 6, y - 9, 1, AMBER, label);

    char row[16], km[12], alt[16];
    int rowY = 160 + (int)i * 26;
    snprintf(row, sizeof(row), "%d %s", (int)i + 1, p.flight);
    formatKm(km, sizeof(km), p.distKm);
    snprintf(alt, sizeof(alt), "%d m", p.altM);
    text(MARGIN, rowY, 2, AMBER, row);
    text(136, rowY + 4, 1, WHITE_TEXT, km);
    textRight(LCD_SIZE - MARGIN, rowY + 4, 1, DIM_TEXT, alt);
  }
}

static void keyValue(int y, const char *key, const char *value) {
  char padded[32];
  snprintf(padded, sizeof(padded), "%18s", value);
  text(MARGIN, y + 3, 1, DIM_TEXT, key);
  textRight(LCD_SIZE - MARGIN, y, 2, WHITE_TEXT, padded + (strlen(padded) > 14 ? strlen(padded) - 14 : 0));
  gfx->drawFastHLine(MARGIN, y + 24, LCD_SIZE - 2 * MARGIN, LINE);
}

static void drawWifi(const Settings &settings, const FeedSnapshot &feed) {
  bool connected = WiFi.status() == WL_CONNECTED;
  const char *state = settings.ssid.isEmpty() ? "Not set      " : connected ? "Connected    " : "Connecting   ";
  gfx->fillCircle(MARGIN + 6, 42, 6, connected ? GREEN : RED);
  text(MARGIN + 20, 34, 2, connected ? GREEN : AMBER, state);

  char buf[24];
  keyValue(66, "Network", settings.ssid.isEmpty() ? "-" : settings.ssid.c_str());
  keyValue(106, "IP", connected ? WiFi.localIP().toString().c_str() : "-");
  snprintf(buf, sizeof(buf), connected ? "%d dBm" : "-", WiFi.RSSI());
  keyValue(146, "Signal", buf);
  if (feed.hasData) snprintf(buf, sizeof(buf), "%lu s ago", (unsigned long)((millis() - feed.lastOkMs) / 1000));
  else strcpy(buf, "-");
  keyValue(186, "Last data", buf);
}

static void drawLocation(const Settings &settings) {
  char buf[24];
  if (settings.hasLocation) {
    snprintf(buf, sizeof(buf), "%.4f", settings.lat);
    keyValue(36, "Latitude", buf);
    snprintf(buf, sizeof(buf), "%.4f", settings.lon);
    keyValue(76, "Longitude", buf);
  } else {
    keyValue(36, "Latitude", "not set");
    keyValue(76, "Longitude", "not set");
  }
  snprintf(buf, sizeof(buf), "%d km", (int)SEARCH_RADIUS_KM);
  keyValue(116, "Search radius", buf);
  text(MARGIN, 170, 1, DIM_TEXT, "Change over USB:");
  text(MARGIN, 186, 1, WHITE_TEXT, "location <lat> <lon>");
}

static const char *titleFor(int screen) {
  static const char *const titles[] = {"OVERHEAD", "NEAREST", "RADAR 20 KM", "WI-FI", "LOCATION"};
  return titles[screen];
}

static void render(bool clear) {
  Settings settings = settingsGet();
  FeedSnapshot feed = feedGet();
  if (clear) {
    gfx->fillScreen(BLACK_BG);
    header(titleFor(currentScreen));
  }

  switch (currentScreen) {
    case SCREEN_WIFI:
      drawWifi(settings, feed);
      break;
    case SCREEN_LOCATION:
      if (clear) drawLocation(settings);
      break;
    default:
      if (clear && !drawEmptyState(settings, feed)) {
        if (currentScreen == SCREEN_ROWS) drawRows(feed);
        else if (currentScreen == SCREEN_CLOSEST) drawClosest(feed);
        else drawRadar(feed);
      }
      drawAge(feed);
      break;
  }
  drawnSequence = feed.sequence;
}

void uiBegin() {
  currentScreen = settingsGet().screen;
  gfx->begin(LCD_SPI_HZ);
  gfx->setTextWrap(false);
  gfx->fillScreen(BLACK_BG);
  ledcAttach(PIN_LCD_BACKLIGHT, 1000, 10);
  ledcWrite(PIN_LCD_BACKLIGHT, BACKLIGHT_DUTY);
}

void uiNextScreen() {
  currentScreen = (currentScreen + 1) % SCREEN_COUNT;
  settingsSetScreen(currentScreen);
  needsFull = true;
}

void uiInvalidate() { needsFull = true; }

void uiTick() {
  uint32_t now = millis();
  static wl_status_t lastWifi = WL_IDLE_STATUS;
  wl_status_t wifi = WiFi.status();
  if (wifi != lastWifi) {
    lastWifi = wifi;
    needsFull = true;
  }

  if (needsFull || feedGet().sequence != drawnSequence) {
    needsFull = false;
    lastSecondMs = now;
    render(true);
  } else if (now - lastSecondMs >= 1000) {
    lastSecondMs = now;
    render(false);
  }
}
