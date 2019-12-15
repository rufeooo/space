#pragma once

#include "math/vec.h"
#include "math/quat.h"

struct ViewComponent {
  ViewComponent() = default;

  ViewComponent(
      const math::Vec3f& position, const math::Quatf& orientation) :
        position(position), orientation(orientation) {}

  math::Vec3f position;
  math::Quatf orientation;
};
