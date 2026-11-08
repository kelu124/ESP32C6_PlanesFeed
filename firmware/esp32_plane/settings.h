// User settings, stored in the NVS data partition so firmware updates keep them.
#pragma once
#include <Arduino.h>

struct Settings {
  String ssid;
  String password;
  bool hasLocation = false;
  double lat = 0;
  double lon = 0;
  int screen = 0;
};

void settingsLoad();

// Returns a copy, safe to call from any task.
Settings settingsGet();

void settingsSetWifi(const String &ssid, const String &password);
void settingsClearWifi();
void settingsSetLocation(double lat, double lon);
void settingsSetScreen(int screen);
