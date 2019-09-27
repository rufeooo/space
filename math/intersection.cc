#include "intersection.h"

#include <iostream>

namespace math {

namespace {

float Signed2DTriArea(
    const math::Vec2f& a, const math::Vec2f& b, const math::Vec2f& c) {
  return (a.x() - c.x()) * (b.y() - c.y()) - 
         (a.y() - c.y()) * (b.x() - c.x());
}


}  // namespace

// pg 152. Real-Time Collision Detection by Christer Ericson
bool LineSegmentsIntersect2D(
    const math::Vec2f& a_start, const math::Vec2f& a_end,
    const math::Vec2f& b_start, const math::Vec2f& b_end) {
  float a1 = Signed2DTriArea(a_start, a_end, b_end);
  float a2 = Signed2DTriArea(a_start, a_end, b_start);
  if (a1 * a2 >= 0.f) return false;
  float a3 = Signed2DTriArea(b_start, b_end, a_start);
  float a4 = a3 + a2 - a1;
  if (a3 * a4 >= 0.f) return false;
  return true;
}

// Calls LineSegmentsIntersect2D (x,y) values from the 3d vectors.
bool LineSegmentsIntersect2D(
    const math::Vec3f& a_start, const math::Vec3f& a_end,
    const math::Vec3f& b_start, const math::Vec3f& b_end) {
  return LineSegmentsIntersect2D(
      math::Vec2f(a_start.x(), a_start.y()),
      math::Vec2f(a_end.x(), a_end.y()),
      math::Vec2f(b_start.x(), b_start.y()),
      math::Vec2f(b_end.x(), a_end.y()));
}

}  // namespace math


