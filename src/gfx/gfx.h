#pragma once

#include "math/mat.h"
#include "math/quat.h"
#include "math/vec.h"

// Do our best to keep all graphics related calls in here.

namespace gfx
{
bool Initialize();

void Render();

void ResetRenderData();

void PushTriangle(const math::Vec3f& position, const math::Vec3f& scale,
                  const math::Quatf& orientation, const math::Vec4f& color);

void PushRectangle(const math::Vec3f& position, const math::Vec3f& scale,
                   const math::Quatf& orientation, const math::Vec4f& color);

void PushCircle(const math::Vec3f& position, const math::Vec3f& scale,
                const math::Quatf& orientation, const math::Vec4f& color);

void PushLine(const math::Vec3f& start, const math::Vec3f& end,
              const math::Vec4f& color);

void PushGrid(float width, float height, const math::Vec4f& color);

void PushAsteroid(const math::Vec3f& position, const math::Vec3f& scale,
                  const math::Quatf& orientation, const math::Vec4f& color);

void PushText(const char* msg, float screen_x, float screen_y);

}  // namespace gfx
