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

  Quat() : Quat(0.0f, math::Vec3<T>(T(0), T(0), T(1))) {}

  void Set(float angle_degrees, const math::Vec3<T>& axis) {
    angle_degrees_ = angle_degrees;
    axis_ = axis;
    axis_.Normalize();
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

  /*Vec3<T> Forward() const {
    return Vec3<T>(
        ,
        ,
        );
  }

  Vec3<T> Up() const {
    return Vec3<T>(
        ,
        ,
        );
  }

  Vec3<T> Left() const {
    return Vec3<T>(
        ,
        ,
        );
  }*/



 private:
  Vec3<T> axis_;
  float angle_degrees_;
};

using Quatf = Quat<float>;

}
