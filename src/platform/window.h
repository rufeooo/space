#pragma once

// Cross-platform window / opengl context abstraction. The purpose
// of these functions are to setup a window on the screen and setup
// the opengl context.
// 
// Notably this module creates a single window and does not intend
// on supporting making multiple.

#include "math/vec.h"

enum PlatformEventType {
  NOT_IMPLEMENTED, // Event does not have translation implemented yet.
  MOUSE_LEFT_UP,
  MOUSE_LEFT_DOWN,
  KEY_DOWN,
  KEY_UP,
};

struct PlatformEvent {
  // Type of event.
  PlatformEventType type;
  // Key pressed in regards to KEY_DOWN, KEY_UP event.
  char key;
  // Screen space the event took place in.
  math::Vec2f position;
};

namespace window
{
void Create(const char* name, int width, int height);

// Returns true if an event existed. False otherwise.
// Fully poll this queue at the top of each game loop.
bool PollEvent(PlatformEvent* event);

void SwapBuffers();

bool ShouldClose();

math::Vec2f GetWindowSize();

math::Vec2f GetCursorPosition();
}  // namespace window
