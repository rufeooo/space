#pragma once

#include <vector>

#include "vec.h"

namespace math {

// Does 2d line segment intersection.
bool LineSegmentsIntersect(
    const math::Vec2f& a_start, const math::Vec2f& a_end,
    const math::Vec2f& b_start, const math::Vec2f& b_end,
    float* time, math::Vec2f* position);

bool PointInPolygon(const math::Vec2f& point,
                    const std::vector<math::Vec2f>& polygon);

}  // namespace math
