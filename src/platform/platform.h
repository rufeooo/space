#pragma once

#include "common/common.h"

// Cross-platform window / opengl context abstraction. The purpose
// of these functions are to setup a window on the screen and setup
// the opengl context.
//
// Notably this module creates a single window and does not intend
// on supporting making multiple.
int platform_init(const char* name, int fullscreen, Rect* window);

void SwapBuffers();

bool ShouldClose();

void GetWindowSize(int* x, int* y);

void GetCursorPosition(int* x, int* y);

// Input?
// Returns true if an event existed. False otherwise.
// Fully poll this queue at the top of each game loop.
// bool PollEvent(PlatformEvent* event);
