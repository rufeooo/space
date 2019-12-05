#pragma once

#include <cmath>

#include "vec.h"

namespace math {

#define PI 3.14159265359

template <typename T>
struct Quat {
  // TODO: This should probably normalize itself because it needs to
  // be a unit vector. Maybe do it before converting to rotation
  // matrix so it's only done when it needs to be.
  
  Quat(const T& w, const T& x, const T& y, const T& z) :
    w(w), x(x), y(y), z(z) {}

  Quat(float angle_degrees, const math::Vec3<T>& axis) {
    Set(angle_degrees, axis);
  }

  Quat() : Quat(0.0f, math::Vec3<T>(T(0), T(0), T(1))) {
    Set(angle_degrees, axis);
  }

  void Set(float new_angle, const math::Vec3<T>& new_axis) {
    angle_degrees = new_angle;
    axis = Normalize(new_axis);
    float angle_radians = (angle_degrees) * PI / 180.0f;
    w = std::cos(angle_radians / 2.0f);
    x = std::sin(angle_radians / 2.0f) * axis.x;
    y = std::sin(angle_radians / 2.0f) * axis.y;
    z = std::sin(angle_radians / 2.0f) * axis.z;
  }

  void Rotate(float angle_degrees_delta) {
    // Wrap the  angle?
    float angle = angle_degrees + angle_degrees_delta;
    Set(angle, axis);
  }

  Vec3<T> Forward() const {
    return Vec3<T>(
        -2.f * x * z + 2.f * w * y,
        -2.f * y * z - 2.f * w * x,
        -1.f + 2.f * x * x + 2.f * y * y);
  }

  Vec3<T> Up() const {
    return Vec3<T>(
        2.f * x * y + 2.f * w * z,
        1.f - 2.f * x * x - 2.f * z * z,
        2.f * y * z - 2.f * w * x);
  }

  Vec3<T> Left() const {
    return Vec3<T>(
        -2.f * x * z + 2.f * w * y,
        -2.f * y * z - 2.f * w * x,
        -1.f + 2.f * x * x + 2.f * y * y);
  }

  T w;
  T x;
  T y;
  T z;

  math::Vec3f axis;
  float angle_degrees;
};

using Quatf = Quat<float>;

}
