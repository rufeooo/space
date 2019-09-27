#pragma once

#include "vec.h"

namespace math {

// Does 2d line segment intersection.
bool LineSegmentsIntersect2D(
    const math::Vec2f& a_start, const math::Vec2f& a_end,
    const math::Vec2f& b_start, const math::Vec2f& b_end);

// Calls LineSegmentsIntersect2D on (xy) with 3d vectors.
bool LineSegmentsIntersect2D(
    const math::Vec3f& a_start, const math::Vec3f& a_end,
    const math::Vec3f& b_start, const math::Vec3f& b_end);


}  // namespace math
