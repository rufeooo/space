#pragma once

#include "vec.h"

namespace math {

struct AxisAlignedRect {
  v3f min; 
  v3f max; 
};

struct Rectf {
  Rectf() = default;
  Rectf(float x, float y, float width, float height) :
    x(x), y(y), width(width), height(height) {}
  float x;
  float y;
  float width;
  float height;
};

// Orients a rect so that it has positive widths and heights.
Rectf
OrientToAabb(const math::Rectf& rect)
{
  Rectf r = rect;
  if (rect.height < 0.f) {
    r.y += rect.height;
    r.height *= -1.f;
  }
  if (rect.width < 0.f) {
    r.x += rect.width;
    r.width *= -1.f;
  }
  return r;
}

}
