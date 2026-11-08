#include "settings.h"

#include <Preferences.h>

#include "config.h"

static Preferences prefs;
static Settings current;
static SemaphoreHandle_t lock;

class Guard {
 public:
  Guard() { xSemaphoreTake(lock, portMAX_DELAY); }
  ~Guard() { xSemaphoreGive(lock); }
};

void settingsLoad() {
  lock = xSemaphoreCreateMutex();
  prefs.begin("plane", false);
  current.ssid = prefs.getString("ssid", "");
  current.password = prefs.getString("pass", "");
  current.hasLocation = prefs.isKey("lat") && prefs.isKey("lon");
  current.lat = prefs.getDouble("lat", 0);
  current.lon = prefs.getDouble("lon", 0);
  current.screen = constrain(prefs.getInt("screen", 0), 0, SCREEN_COUNT - 1);
}

Settings settingsGet() {
  Guard guard;
  return current;
}

void settingsSetWifi(const String &ssid, const String &password) {
  Guard guard;
  prefs.putString("ssid", ssid);
  prefs.putString("pass", password);
  current.ssid = ssid;
  current.password = password;
}

void settingsClearWifi() {
  Guard guard;
  prefs.remove("ssid");
  prefs.remove("pass");
  current.ssid = "";
  current.password = "";
}

void settingsSetLocation(double lat, double lon) {
  Guard guard;
  prefs.putDouble("lat", lat);
  prefs.putDouble("lon", lon);
  current.lat = lat;
  current.lon = lon;
  current.hasLocation = true;
}

void settingsSetScreen(int screen) {
  Guard guard;
  if (screen == current.screen) return;
  prefs.putInt("screen", screen);
  current.screen = screen;
}
