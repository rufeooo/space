#pragma once

#include "math/vec.h"
#include "math/mat.h"

// Do our best to keep all graphics related calls in here.

namespace gfx
{

// Overwrite for custom projection matrix.
typedef math::Mat4f CreateProjectionFunctor(void);

void SetProjection(CreateProjectionFunctor* projection);

bool Initialize();

void Render();

}  // namespace gfx
