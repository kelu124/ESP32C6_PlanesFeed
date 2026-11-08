// Host tests for firmware/esp32_plane/logic.cpp. Run with: make test
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../firmware/esp32_plane/logic.h"

static int failures = 0;

#define CHECK(cond)                                         \
  do {                                                      \
    if (!(cond)) {                                          \
      printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                           \
    }                                                       \
  } while (0)

static Plane planeAt(const char *flight, float km) {
  Plane p = {};
  strncpy(p.flight, flight, sizeof(p.flight) - 1);
  p.distKm = km;
  return p;
}

static void testDistance() {
  // One degree of latitude is about 111.19 km.
  CHECK(fabs(distanceKm(48.0, 2.0, 49.0, 2.0) - 111.19) < 0.05);
  CHECK(distanceKm(48.88, 2.35, 48.88, 2.35) < 0.001);
}

static void testBearing() {
  CHECK(fabs(bearingDeg(48.0, 2.0, 49.0, 2.0) - 0.0) < 0.1);
  CHECK(fabs(bearingDeg(48.0, 2.0, 47.0, 2.0) - 180.0) < 0.1);
  CHECK(fabs(bearingDeg(0.0, 0.0, 0.0, 1.0) - 90.0) < 0.1);
  CHECK(fabs(bearingDeg(0.0, 1.0, 0.0, 0.0) - 270.0) < 0.1);
}

static void testInsertClosest() {
  Plane top[TOP_PLANES];
  size_t count = 0;
  insertClosest(top, TOP_PLANES, count, planeAt("C", 14));
  insertClosest(top, TOP_PLANES, count, planeAt("A", 4));
  insertClosest(top, TOP_PLANES, count, planeAt("D", 18));
  insertClosest(top, TOP_PLANES, count, planeAt("B", 9));
  insertClosest(top, TOP_PLANES, count, planeAt("E", 19));
  CHECK(count == 3);
  CHECK(strcmp(top[0].flight, "A") == 0);
  CHECK(strcmp(top[1].flight, "B") == 0);
  CHECK(strcmp(top[2].flight, "C") == 0);
}

static void testCompass() {
  CHECK(strcmp(compassName(0), "N") == 0);
  CHECK(strcmp(compassName(44), "NE") == 0);
  CHECK(strcmp(compassName(90), "E") == 0);
  CHECK(strcmp(compassName(359), "N") == 0);
  CHECK(strcmp(compassName(-90), "W") == 0);
}

static void testCopyTrimmed() {
  char out[10];
  copyTrimmed(out, sizeof(out), "  AFR15EZ ");
  CHECK(strcmp(out, "AFR15EZ") == 0);
  copyTrimmed(out, 4, "ABCDEFG");
  CHECK(strcmp(out, "ABC") == 0);
  copyTrimmed(out, sizeof(out), NULL);
  CHECK(out[0] == '\0');
}

static void testFormatRoute() {
  char out[48];
  formatRoute(out, sizeof(out), 20, "Porto", "OPO", "Eindhoven", "EIN");
  CHECK(strcmp(out, "Porto > Eindhoven") == 0);
  formatRoute(out, sizeof(out), 20, "San Francisco", "SFO", "Amsterdam", "AMS");
  CHECK(strcmp(out, "SFO > AMS") == 0);
  formatRoute(out, sizeof(out), 20, "M\xC3\xA1laga", "AGP", "Paris", "CDG");
  CHECK(strcmp(out, "AGP > CDG") == 0);
}

static void testTokenize() {
  char *argv[4];

  char a[] = "wifi MyNet secret";
  CHECK(tokenize(a, argv, 4) == 3);
  CHECK(strcmp(argv[1], "MyNet") == 0);
  CHECK(strcmp(argv[2], "secret") == 0);

  char b[] = "wifi \"My Home Net\" \"pa ss\\\"wo\\\\rd\"";
  CHECK(tokenize(b, argv, 4) == 3);
  CHECK(strcmp(argv[1], "My Home Net") == 0);
  CHECK(strcmp(argv[2], "pa ss\"wo\\rd") == 0);

  char c[] = "   ";
  CHECK(tokenize(c, argv, 4) == 0);

  char d[] = "wifi \"unterminated";
  CHECK(tokenize(d, argv, 4) == -1);

  char e[] = "a b c d e";
  CHECK(tokenize(e, argv, 4) == -1);

  char f[] = "wifi \"\"";
  CHECK(tokenize(f, argv, 4) == 2);
  CHECK(argv[1][0] == '\0');
}

static void testParseCoordinate() {
  double v = 0;
  CHECK(parseCoordinate("48.8566", -90, 90, v) && fabs(v - 48.8566) < 1e-9);
  CHECK(parseCoordinate("-2.5", -180, 180, v) && v == -2.5);
  CHECK(!parseCoordinate("91", -90, 90, v));
  CHECK(!parseCoordinate("48.8abc", -90, 90, v));
  CHECK(!parseCoordinate("", -90, 90, v));
  CHECK(!parseCoordinate("nan", -90, 90, v));
}

int main() {
  testDistance();
  testBearing();
  testInsertClosest();
  testCompass();
  testCopyTrimmed();
  testFormatRoute();
  testTokenize();
  testParseCoordinate();
  if (failures == 0) printf("All logic tests passed\n");
  return failures == 0 ? 0 : 1;
}
