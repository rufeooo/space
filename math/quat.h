#pragma once

#include "vec.h"

#include <cmath>

namespace math {

#define PI 3.14159265359

template <typename T>
class Quat : public Vec4<T> {
 public:

  // TODO: This should probably normalize itself because it needs to
  // be a unit vector. Maybe do it before converting to rotation
  // matrix so it's only done when it needs to be.
  

  Quat(float angle_degrees, const math::Vec3<T>& axis) {
    Set(angle_degrees, axis);
  }

  Quat() : Quat(90.0f, math::Vec3<T>(T(1), T(0), T(0))) {}

  void Set(float angle_degrees, const math::Vec3<T>& axis) {
    angle_degrees_ = angle_degrees;
    axis_ = axis;
    float angle_radians = (angle_degrees) * PI / 180.0f;
    this->data_[0] = std::cos(angle_radians / 2.0f);
    this->data_[1] = std::sin(angle_radians / 2.0f) * axis.x();
    this->data_[2] = std::sin(angle_radians / 2.0f) * axis.y();
    this->data_[3] = std::sin(angle_radians / 2.0f) * axis.z();
  }

  void Rotate(float angle_degrees_delta) {
    // Wrap the  angle?
    float angle = angle_degrees_ + angle_degrees_delta;
    Set(angle_degrees_ + angle_degrees_delta, axis_);
  }

  Vec3<T> Forward() const {
    return Vec3<T>(
        2.f * (this->x() * this->z() + this->w() * this->y()),
        2.f * (this->y() * this->z() - this->w() * this->x()),
        1.f - 2.f * (this->x() * this->x() + this->y() * this->y()));
  }

  Vec3<T> Up() const {
    return Vec3<T>(
        2.f * (this->x() * this->y() + this->w() * this->z()),
        1.f - 2.f * (this->x() * this->x() + this->z() * this->z()),
        2.f * (this->y() * this->z() + this->w() * this->x()));
  }

  Vec3<T> Left() const {
    return Vec3<T>(
        1.f - 2.f * (this->y() * this->y() + this->z() * this->z()),
        2.f * (this->x() * this->y() + this->w() * this->z()),
        2.f * (this->x() * this->z() - this->w() * this->y()));
  }



 private:
  Vec3<T> axis_;
  float angle_degrees_;
};

using Quatf = Quat<float>;

}
