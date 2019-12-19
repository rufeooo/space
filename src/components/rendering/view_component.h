#pragma once

#include "math/quat.h"
#include "math/vec.h"

struct ViewComponent {
  ViewComponent() = default;

  ViewComponent(const math::Vec3f& position, const math::Quatf& orientation)
      : position(position), orientation(orientation)
  {
  }

  math::Vec3f position;
  math::Quatf orientation;
};
