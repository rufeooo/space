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

void PushTriangle(const math::Vec3f& position, const math::Vec3f& scale,
                  const math::Quatf& orientation, const math::Vec4f& color);

void PushRectangle(const math::Vec3f& position, const math::Vec3f& scale,
                   const math::Quatf& orientation, const math::Vec4f& color);

void PushText(const char* msg, float screen_x, float screen_y);

}  // namespace gfx
