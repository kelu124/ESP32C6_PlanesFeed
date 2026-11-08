// Background task that fetches nearby aircraft and the closest flight's route.
#pragma once
#include <Arduino.h>

#include "logic.h"

struct FeedSnapshot {
  Plane planes[TOP_PLANES];
  size_t count = 0;
  bool hasData = false;
  uint32_t lastOkMs = 0;
  uint32_t sequence = 0;  // increases each time the displayed data changes
  char route[48] = "";
  int lastHttp = 0;
};

void feedStart();
FeedSnapshot feedGet();

// Asks for a fetch now, for example after the location changes.
void feedWake();
