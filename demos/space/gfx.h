#pragma once

#include "math/vec.h"

// Do our best to keep all graphics related calls in here.

namespace gfx {

bool Initialize();

// Poll OS specific events.
void PollEvents();

bool Render();

math::Vec2f GetCursorPositionInScreenSpace();

math::Vec2f GetCursorPositionInGLSpace();

math::Vec2f GetCursorPositionInWorldSpace();

math::Vec2f GetWindowDims();

bool LeftMouseClicked();

}
