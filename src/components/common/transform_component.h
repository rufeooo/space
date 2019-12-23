#pragma once

#include "math/quat.h"
#include "math/vec.h"

struct TransformComponent {
  math::Vec3f position;
  math::Vec3f scale = math::Vec3f(1.f, 1.f, 1.f);
  math::Quatf orientation;
  math::Vec3f prev_position;
};
