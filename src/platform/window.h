#pragma once

// Cross-platform window / opengl context abstraction. The purpose
// of these functions are to setup a window on the screen and setup
// the opengl context.
// 
// Notably this module creates a single window and does not intend
// on supporting making multiple.

#include "math/vec.h"

enum Key {
  KEY_NONE = 0,
  KEY_W    = 0x1,
  KEY_A    = 0x2,
  KEY_S    = 0x4,
  KEY_D    = 0x8,
};

namespace window
{
void Create(const char* name, int width, int height);

void PollEvents();

void SwapBuffers();

bool ShouldClose();

math::Vec2f GetWindowSize();

bool IsKeyDown(Key key);
}  // namespace window
