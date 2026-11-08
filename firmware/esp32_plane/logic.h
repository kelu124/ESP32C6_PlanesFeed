// Board-independent logic. Compiled into the firmware and into test/test_logic.cpp.
#pragma once
#include <stddef.h>

static const size_t TOP_PLANES = 3;

struct Plane {
  char flight[10];
  char type[6];
  char desc[28];
  int altM;
  int speedKmh;
  int trackDeg;
  float distKm;      // horizontal distance from the point directly overhead
  float bearingDeg;  // direction from the observer to the plane
};

// Great-circle distance in km between two points, in decimal degrees.
float distanceKm(double lat1, double lon1, double lat2, double lon2);

// Initial bearing in degrees (0 to 360) from point 1 to point 2.
float bearingDeg(double lat1, double lon1, double lat2, double lon2);

// Inserts candidate into top, kept sorted by distKm and capped at maxTop.
// count is the number of entries in use and is updated.
void insertClosest(Plane *top, size_t maxTop, size_t &count, const Plane &candidate);

// Eight-point compass name for a heading in degrees.
const char *compassName(int deg);

// Copies src to dst without surrounding spaces, always NUL-terminating.
void copyTrimmed(char *dst, size_t dstSize, const char *src);

// Writes "City > City" when it fits in maxChars and both names are plain ASCII,
// otherwise "AAA > BBB" using the airport codes.
void formatRoute(char *out, size_t outSize, size_t maxChars, const char *fromCity,
                 const char *fromCode, const char *toCity, const char *toCode);

// Splits line in place on spaces. Double quotes group words, and \" or \\ inside
// quotes give a literal character. Returns the number of tokens, or -1 on an
// unterminated quote or more than maxArgs tokens.
int tokenize(char *line, char **argv, int maxArgs);

// Strict decimal parser: returns false unless all of text is a number in [min, max].
bool parseCoordinate(const char *text, double min, double max, double &out);
