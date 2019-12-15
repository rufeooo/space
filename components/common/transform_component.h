#pragma once

#include "math/vec.h"
#include "math/quat.h"

struct TransformComponent {
  math::Vec3f position;
  math::Quatf orientation;
  math::Vec3f prev_position;
};
