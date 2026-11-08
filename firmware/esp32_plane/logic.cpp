#include "logic.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const double EARTH_RADIUS_KM = 6371.0;
static const double DEG_TO_RADIANS = M_PI / 180.0;

float distanceKm(double lat1, double lon1, double lat2, double lon2) {
  double phi1 = lat1 * DEG_TO_RADIANS;
  double phi2 = lat2 * DEG_TO_RADIANS;
  double dPhi = phi2 - phi1;
  double dLambda = (lon2 - lon1) * DEG_TO_RADIANS;
  double a = sin(dPhi / 2) * sin(dPhi / 2) +
             cos(phi1) * cos(phi2) * sin(dLambda / 2) * sin(dLambda / 2);
  return 2 * EARTH_RADIUS_KM * atan2(sqrt(a), sqrt(1 - a));
}

float bearingDeg(double lat1, double lon1, double lat2, double lon2) {
  double phi1 = lat1 * DEG_TO_RADIANS;
  double phi2 = lat2 * DEG_TO_RADIANS;
  double dLambda = (lon2 - lon1) * DEG_TO_RADIANS;
  double y = sin(dLambda) * cos(phi2);
  double x = cos(phi1) * sin(phi2) - sin(phi1) * cos(phi2) * cos(dLambda);
  double deg = atan2(y, x) / DEG_TO_RADIANS;
  return deg < 0 ? deg + 360.0 : deg;
}

void insertClosest(Plane *top, size_t maxTop, size_t &count, const Plane &candidate) {
  size_t pos = 0;
  while (pos < count && top[pos].distKm <= candidate.distKm) pos++;
  if (pos >= maxTop) return;
  size_t last = count < maxTop ? count : maxTop - 1;
  for (size_t i = last; i > pos; i--) top[i] = top[i - 1];
  top[pos] = candidate;
  if (count < maxTop) count++;
}

const char *compassName(int deg) {
  static const char *const names[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
  int normalized = ((deg % 360) + 360) % 360;
  return names[((normalized * 2 + 45) / 90) % 8];
}

void copyTrimmed(char *dst, size_t dstSize, const char *src) {
  if (dstSize == 0) return;
  dst[0] = '\0';
  if (src == NULL) return;
  while (*src == ' ') src++;
  size_t len = strlen(src);
  while (len > 0 && src[len - 1] == ' ') len--;
  if (len >= dstSize) len = dstSize - 1;
  memcpy(dst, src, len);
  dst[len] = '\0';
}

static bool isPlainAscii(const char *text) {
  if (text == NULL || *text == '\0') return false;
  for (; *text; text++) {
    if ((unsigned char)*text < 0x20 || (unsigned char)*text > 0x7E) return false;
  }
  return true;
}

void formatRoute(char *out, size_t outSize, size_t maxChars, const char *fromCity,
                 const char *fromCode, const char *toCity, const char *toCode) {
  if (isPlainAscii(fromCity) && isPlainAscii(toCity) &&
      strlen(fromCity) + strlen(toCity) + 3 <= maxChars) {
    snprintf(out, outSize, "%s > %s", fromCity, toCity);
    return;
  }
  snprintf(out, outSize, "%s > %s", fromCode ? fromCode : "?", toCode ? toCode : "?");
}

int tokenize(char *line, char **argv, int maxArgs) {
  int argc = 0;
  char *read = line;
  while (*read) {
    while (*read == ' ' || *read == '\t') read++;
    if (*read == '\0') break;
    if (argc == maxArgs) return -1;

    char *write = read;
    argv[argc++] = write;
    bool quoted = false;
    while (*read && (quoted || (*read != ' ' && *read != '\t'))) {
      if (*read == '"') {
        quoted = !quoted;
        read++;
      } else if (quoted && *read == '\\' && (read[1] == '"' || read[1] == '\\')) {
        *write++ = read[1];
        read += 2;
      } else {
        *write++ = *read++;
      }
    }
    if (quoted) return -1;
    bool atEnd = *read == '\0';
    *write = '\0';
    if (atEnd) break;
    read++;
  }
  return argc;
}

bool parseCoordinate(const char *text, double min, double max, double &out) {
  if (text == NULL || *text == '\0') return false;
  char *end = NULL;
  double value = strtod(text, &end);
  if (end == text || *end != '\0' || !isfinite(value)) return false;
  if (value < min || value > max) return false;
  out = value;
  return true;
}
