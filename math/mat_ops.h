#pragma once

#include "mat.h"
#include "vec.h"

namespace math {

template <class T, size_t N>
Mat<T, N, N> CreateIdentityMatrix() {
  Mat<T, N, N> m;
  // Matrices don't 0 initializate. Maybe they should.
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      m(i, j) = i == j
          ? static_cast<T>(1)
          : static_cast<T>(0);
    }
  }
  return m;
}

template <class T>
Mat<T, 4, 4> CreateTranslationMatrix(const Vec3<T>& translation) {
  Mat<T, 4, 4> m = CreateIdentityMatrix<T, 4>();
  m[12] = translation.x();
  m[13] = translation.y();
  m[14] = translation.z();
  return m;
}

template <class T>
Mat<T, 4, 4> CreateRotationMatrix(const Quat<T>& quat) {
  T x = quat.x(); T x2 = x * x;
  T y = quat.y(); T y2 = y * y;
  T z = quat.z(); T z2 = z * z;
  T w = quat.w();
  Mat<T, 4, 4> rot;
  rot[0]  = 1.f - 2.f * y2 - 2.f * z2;
  rot[1]  = 2.f * x * y - 2.f * w * z;
  rot[2]  = 2.f * x * z + 2.f * w * y;
  rot[3]  = static_cast<T>(0);
  rot[4]  = 2.f * x * y + 2.f * w * z;
  rot[5]  = 1.f - 2.f * x2 - 2.f * z2;
  rot[6]  = 2.f * y * z - 2.f * w * x;
  rot[7]  = static_cast<T>(0);
  rot[8]  = 2.f * x * z - 2.f * w * y;
  rot[9]  = 2.f * y * z + 2.f * w * x;
  rot[10] = 1.f - 2.f * x2 - 2.f * y2;
  rot[11] = static_cast<T>(0);
  rot[12] = static_cast<T>(0);
  rot[13] = static_cast<T>(0);
  rot[14] = static_cast<T>(0);
  rot[15] = static_cast<T>(1);
  return rot;
}

}  // math
