// Shows the planes closest to overhead on a Waveshare ESP32-C6-LCD-1.3.
// Setup is done over the USB serial CLI: type "help". See docs/design.md.
#include <WiFi.h>
#include <esp_system.h>
#include <soc/lp_aon_reg.h>

#include "config.h"
#include "feed.h"
#include "logic.h"
#include "settings.h"
#include "ui.h"

static const uint32_t DEBOUNCE_MS = 30;
static const size_t CLI_LINE_MAX = 200;

static void connectWifi() {
  Settings settings = settingsGet();
  WiFi.disconnect();
  if (settings.ssid.isEmpty()) return;
  WiFi.begin(settings.ssid.c_str(), settings.password.c_str());
}

static void printStatus() {
  Settings settings = settingsGet();
  FeedSnapshot feed = feedGet();
  bool connected = WiFi.status() == WL_CONNECTED;
  Serial.printf("wifi.ssid: %s\n", settings.ssid.isEmpty() ? "(not set)" : settings.ssid.c_str());
  Serial.printf("wifi.password: %s\n", settings.password.isEmpty() ? "(not set)" : "(saved)");
  Serial.printf("wifi.state: %s\n", connected ? "connected" : "disconnected");
  if (connected) {
    Serial.printf("wifi.ip: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("wifi.rssi: %d dBm\n", WiFi.RSSI());
  }
  if (settings.hasLocation) Serial.printf("location: %.6f %.6f\n", settings.lat, settings.lon);
  else Serial.println("location: (not set)");
  Serial.printf("feed.last_http: %d\n", feed.lastHttp);
  if (feed.hasData) {
    Serial.printf("feed.age_s: %lu\n", (unsigned long)((millis() - feed.lastOkMs) / 1000));
    for (size_t i = 0; i < feed.count; i++) {
      const Plane &p = feed.planes[i];
      Serial.printf("plane.%u: %s %s %.1f km %d m\n", (unsigned)i + 1, p.flight, p.type, p.distKm, p.altM);
    }
    if (feed.count > 0) Serial.printf("route: %s\n", feed.route);
  }
  Serial.printf("heap.free: %lu\n", (unsigned long)ESP.getFreeHeap());
}

static void printHelp() {
  Serial.println("help                       show this list");
  Serial.println("status                     show Wi-Fi, location and plane data");
  Serial.println("wifi <name> [password]     save Wi-Fi and connect; quote values with spaces");
  Serial.println("wifi clear                 forget the saved Wi-Fi");
  Serial.println("location <lat> <lon>       save the location in decimal degrees");
  Serial.println("reboot                     restart normally");
  Serial.println("dfu                        restart into firmware download mode");
}

static void enterDownloadMode() {
  Serial.println("ok: restarting into download mode; to leave, run make leave-dfu or unplug the board");
  Serial.flush();
  delay(100);
  REG_SET_BIT(LP_AON_SYS_CFG_REG, LP_AON_FORCE_DOWNLOAD_BOOT);
  esp_restart();
}

static void runCommand(char *line) {
  char *argv[4];
  int argc = tokenize(line, argv, 4);
  if (argc == 0) return;
  if (argc < 0) {
    Serial.println("error: too many words or an unclosed quote");
    return;
  }

  String cmd = argv[0];
  if (cmd == "help") {
    printHelp();
  } else if (cmd == "status") {
    printStatus();
  } else if (cmd == "reboot") {
    Serial.println("ok: restarting");
    Serial.flush();
    delay(100);
    esp_restart();
  } else if (cmd == "dfu") {
    enterDownloadMode();
  } else if (cmd == "wifi" && argc == 2 && String(argv[1]) == "clear") {
    settingsClearWifi();
    connectWifi();
    uiInvalidate();
    Serial.println("ok: wifi cleared");
  } else if (cmd == "wifi" && (argc == 2 || argc == 3)) {
    if (argv[1][0] == '\0') {
      Serial.println("error: the network name is empty");
      return;
    }
    settingsSetWifi(argv[1], argc == 3 ? argv[2] : "");
    connectWifi();
    uiInvalidate();
    Serial.println("ok: wifi saved, connecting");
  } else if (cmd == "location" && argc == 3) {
    double lat, lon;
    if (!parseCoordinate(argv[1], -90, 90, lat) || !parseCoordinate(argv[2], -180, 180, lon)) {
      Serial.println("error: expected location <lat -90..90> <lon -180..180>");
      return;
    }
    settingsSetLocation(lat, lon);
    uiInvalidate();
    feedWake();
    Serial.printf("ok: location saved: %.6f %.6f\n", lat, lon);
  } else {
    Serial.println("error: unknown command or wrong arguments, type help");
  }
}

static void pollCli() {
  static char line[CLI_LINE_MAX + 1];
  static size_t length = 0;
  static bool overflow = false;

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (overflow) {
        Serial.println("error: line too long");
      } else if (length > 0) {
        line[length] = '\0';
        runCommand(line);
      }
      length = 0;
      overflow = false;
    } else if (length < CLI_LINE_MAX) {
      line[length++] = c;
    } else {
      overflow = true;
    }
  }
}

static void pollButton() {
  static bool stable = HIGH, lastRead = HIGH;
  static uint32_t changedMs = 0;
  bool reading = digitalRead(PIN_BUTTON);
  uint32_t now = millis();
  if (reading != lastRead) {
    lastRead = reading;
    changedMs = now;
  }
  if (reading != stable && now - changedMs >= DEBOUNCE_MS) {
    stable = reading;
    if (stable == LOW) uiNextScreen();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  settingsLoad();
  uiBegin();
  WiFi.persistent(false);  // credentials live only in our own settings
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);  // power saving delays traffic enough to make HTTPS connections time out
  WiFi.setAutoReconnect(true);
  connectWifi();
  feedStart();
  Serial.println("esp32_plane ready, type help");
}

void loop() {
  pollCli();
  pollButton();
  uiTick();
  delay(5);
}
