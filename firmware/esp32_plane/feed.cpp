#include "feed.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "config.h"
#include "settings.h"

static const char USER_AGENT[] = "esp32-plane/1.0";
static const uint32_t HTTP_TIMEOUT_MS = 8000;
static const int ROUTE_TEXT_CHARS = 20;  // width of the route line on the LCD

static SemaphoreHandle_t lock;
static TaskHandle_t task;
static FeedSnapshot shared;

static void publish(const FeedSnapshot &next) {
  xSemaphoreTake(lock, portMAX_DELAY);
  shared = next;
  xSemaphoreGive(lock);
}

FeedSnapshot feedGet() {
  xSemaphoreTake(lock, portMAX_DELAY);
  FeedSnapshot copy = shared;
  xSemaphoreGive(lock);
  return copy;
}

void feedWake() {
  if (task) xTaskNotifyGive(task);
}

// The feed data is public and no credentials are sent, so the server certificate
// is not verified. See docs/design.md.
static int openJson(HTTPClient &http, WiFiClientSecure &client, const String &url) {
  client.setInsecure();
  client.setTimeout(HTTP_TIMEOUT_MS / 1000);
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.useHTTP10(true);  // no chunked encoding, so ArduinoJson can read the stream
  http.setUserAgent(USER_AGENT);
  if (!http.begin(client, url)) return -1;
  return http.GET();
}

static bool toPlane(JsonObjectConst ac, const Settings &where, Plane &out) {
  if (!ac["lat"].is<double>() || !ac["lon"].is<double>()) return false;
  // alt_baro is the string "ground" for taxiing aircraft, which are skipped.
  if (!ac["alt_baro"].is<float>()) return false;

  double lat = ac["lat"];
  double lon = ac["lon"];
  float km = distanceKm(where.lat, where.lon, lat, lon);
  if (km > SEARCH_RADIUS_KM) return false;

  memset(&out, 0, sizeof(out));
  copyTrimmed(out.flight, sizeof(out.flight), ac["flight"] | "");
  if (!out.flight[0]) copyTrimmed(out.flight, sizeof(out.flight), ac["r"] | "");
  if (!out.flight[0]) copyTrimmed(out.flight, sizeof(out.flight), ac["hex"] | "?");
  copyTrimmed(out.type, sizeof(out.type), ac["t"] | "?");
  copyTrimmed(out.desc, sizeof(out.desc), ac["desc"] | out.type);
  out.altM = lroundf(ac["alt_baro"].as<float>() * 0.3048f);
  out.speedKmh = lroundf((ac["gs"] | 0.0f) * 1.852f);
  out.trackDeg = lroundf(ac["track"] | 0.0f);
  out.distKm = km;
  out.bearingDeg = bearingDeg(where.lat, where.lon, lat, lon);
  return true;
}

static int fetchPlanes(const Settings &where, FeedSnapshot &next) {
  WiFiClientSecure client;
  HTTPClient http;
  String url = "https://opendata.adsb.fi/api/v3/lat/" + String(where.lat, 6) + "/lon/" +
               String(where.lon, 6) + "/dist/" + String(SEARCH_RADIUS_NM);
  int status = openJson(http, client, url);
  if (status != HTTP_CODE_OK) {
    http.end();
    return status;
  }

  JsonDocument filter;
  JsonObject item = filter["ac"].add<JsonObject>();
  for (const char *key : {"flight", "r", "hex", "t", "desc", "alt_baro", "gs", "track", "lat", "lon"}) {
    item[key] = true;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
  http.end();
  if (err) {
    Serial.printf("feed: bad JSON: %s\n", err.c_str());
    return -2;
  }

  next.count = 0;
  for (JsonObjectConst ac : doc["ac"].as<JsonArrayConst>()) {
    Plane plane;
    if (toPlane(ac, where, plane)) insertClosest(next.planes, TOP_PLANES, next.count, plane);
  }
  return status;
}

// Returns false on a temporary failure, so the lookup is retried on the next cycle.
static bool fetchRoute(const char *flight, char *route, size_t routeSize) {
  for (const char *c = flight; *c; c++) {
    if (!isalnum((unsigned char)*c)) {
      snprintf(route, routeSize, "Route unknown");
      return true;
    }
  }

  WiFiClientSecure client;
  HTTPClient http;
  int status = openJson(http, client, String("https://api.adsbdb.com/v0/callsign/") + flight);
  if (status == HTTP_CODE_NOT_FOUND) {
    http.end();
    snprintf(route, routeSize, "Route unknown");
    return true;
  }
  if (status != HTTP_CODE_OK) {
    http.end();
    return false;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, http.getStream());
  http.end();
  if (err) return false;

  JsonObjectConst flightRoute = doc["response"]["flightroute"];
  if (flightRoute.isNull()) {
    snprintf(route, routeSize, "Route unknown");
    return true;
  }
  formatRoute(route, routeSize, ROUTE_TEXT_CHARS, flightRoute["origin"]["municipality"],
              flightRoute["origin"]["iata_code"], flightRoute["destination"]["municipality"],
              flightRoute["destination"]["iata_code"]);
  return true;
}

static void feedTask(void *) {
  FeedSnapshot next;
  char routeFlight[sizeof(Plane::flight)] = "";
  uint32_t waitMs = 0;

  for (;;) {
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(waitMs));
    waitMs = FEED_INTERVAL_MS;

    Settings settings = settingsGet();
    if (WiFi.status() != WL_CONNECTED || !settings.hasLocation) continue;

    int status = fetchPlanes(settings, next);
    next.lastHttp = status;
    if (status == HTTP_CODE_TOO_MANY_REQUESTS) waitMs = FEED_BACKOFF_MS;
    if (status != HTTP_CODE_OK) {
      Serial.printf("feed: request failed: %d\n", status);
      FeedSnapshot failed = feedGet();
      failed.lastHttp = status;
      publish(failed);
      continue;
    }

    if (next.count == 0) {
      routeFlight[0] = '\0';
      next.route[0] = '\0';
    } else if (strcmp(routeFlight, next.planes[0].flight) != 0) {
      if (fetchRoute(next.planes[0].flight, next.route, sizeof(next.route))) {
        memcpy(routeFlight, next.planes[0].flight, sizeof(routeFlight));
      } else {
        snprintf(next.route, sizeof(next.route), "Route unknown");
      }
    }

    next.hasData = true;
    next.lastOkMs = millis();
    next.sequence++;
    publish(next);
  }
}

void feedStart() {
  lock = xSemaphoreCreateMutex();
  xTaskCreate(feedTask, "feed", 16384, NULL, 1, &task);
}
