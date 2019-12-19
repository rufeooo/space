#pragma once

#include "math/vec.h"

// Do our best to keep all graphics related calls in here.

namespace gfx
{
enum Input {
  KEY_NONE = 0,
  KEY_W = 0x1,
  KEY_A = 0x2,
  KEY_S = 0x4,
  KEY_D = 0x8,
  MOUSE_LEFT_CLICK = 0x10,
};

bool Initialize();

// Poll OS specific events.
void PollEvents();

bool Render();

math::Vec2f GetCursorPositionInScreenSpace();

math::Vec2f GetCursorPositionInWorldSpace();

math::Vec2f GetWindowDims();

bool LeftMouseClicked();

// Contains a 32 bit integer containing keys that are
// currently being pressed.
uint32_t GetInputMask();

}  // namespace gfx
