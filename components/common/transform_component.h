#pragma once

#include "math/quat.h"
#include "math/vec.h"

struct TransformComponent {
  math::Vec3f position;
  math::Quatf orientation;
  math::Vec3f prev_position;
};
