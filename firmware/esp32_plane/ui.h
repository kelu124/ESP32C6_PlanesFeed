// Draws the five LCD screens.
#pragma once

void uiBegin();
void uiNextScreen();

// Call often from the main loop. Redraws only when something changed.
void uiTick();

// Forces a redraw, for example after a setting changes.
void uiInvalidate();
