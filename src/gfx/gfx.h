#pragma once

#include "math/mat.h"
#include "math/quat.h"
#include "math/vec.h"

#include "platform/platform.cc"

// Do our best to keep all graphics related calls in here.

namespace gfx
{
bool Initialize();

void Render();

void PushText(const char* msg, float screen_x, float screen_y);
}  // namespace gfx
