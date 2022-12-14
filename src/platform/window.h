#pragma once

#include "math/vec.h"

// Cross-platform window / opengl context abstraction. The purpose
// of these functions are to setup a window on the screen and setup
// the opengl context.
//
// Notably this module creates a single window and does not intend
// on supporting making multiple.

enum PlatformEventType {
  NOT_IMPLEMENTED,  // Event does not have translation implemented yet.
  MOUSE_DOWN,
  MOUSE_UP,
  MOUSE_WHEEL,
  KEY_DOWN,
  KEY_UP,
  MOUSE_POSITION,
};

enum PlatformButton {
  BUTTON_UNKNOWN,
  BUTTON_LEFT = 1,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
};

struct PlatformEvent {
  // Type of event.
  PlatformEventType type;
  // Screen space the event took place in.
  v2f position;
  // Event Detail
  union {
    float wheel_delta;
    char key;
    PlatformButton button;
  };
};

namespace window
{

struct CreateInfo {
  uint64_t window_width = 1920;
  uint64_t window_height = 1080;
  // If left as UINT64_MAX let the platform decide where the window should go.
  uint64_t window_pos_x = UINT64_MAX;
  uint64_t window_pos_y = UINT64_MAX;
  bool fullscreen = false;
};

int Create(const char* name, int width, int height, bool fullscreen);

int Create(const char* name, const CreateInfo& create_info);

// Returns true if an event existed. False otherwise.
// Fully poll this queue at the top of each game loop.
bool PollEvent(PlatformEvent* event);

void SwapBuffers();

bool ShouldClose();

v2f GetWindowSize();

v2f GetCursorPosition();
}  // namespace window
