#pragma once

#include "mat.h"
#include "vec.h"
#include "quat.h"

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
  Mat<T, 4, 4> rotation;
  auto w = quat.x();
  auto x = quat.y();
  auto y = quat.z();
  auto z = quat.w();
  rotation[0]  = 1.f - 2.f * y * y - 2.f * z * z;
  rotation[1]  = 2.f * x * y - 2.f * w * z;
  rotation[2]  = 2.f * x * z + 2 * w * y;
  rotation[3]  = T(0);
  rotation[4]  = 2.f * x * y + 2.f * w * z;
  rotation[5]  = 1.f - 2.f * x * x - 2.f * z * z;
  rotation[6]  = 2.f * y * z - 2.f * w * x;
  rotation[7]  = T(0);
  rotation[8]  = 2.f * x * z - 2.f * w * y;
  rotation[9]  = 2.f * y * z + 2.f * w * x;
  rotation[10] = 1.f - 2.f * x * x - 2.f * y * y;
  rotation[11] = T(0);
  rotation[12] = T(0);
  rotation[13] = T(0);
  rotation[14] = T(0);
  rotation[15] = T(1);
  return rotation;
}

template <class T>
Mat<T, 4, 4> CreateViewMatrix(const Vec3<T>& translation,
                              const Quat<T>& quat) {
  Mat<T, 4, 4> view;
  // A quat is really a versor [w,(x,y,z)]. So grab the variables out
  // with those names to make the view more clear.
  auto w = quat.x();
  auto x = quat.y();
  auto y = quat.z();
  auto z = quat.w();
  view[0]  = 1.f - 2.f * y * y - 2.f * z * z;
  view[1]  = 2.f * x * y - 2.f * w * z;
  view[2]  = 2.f * x * z + 2 * w * y;
  view[3]  = -translation.x();
  view[4]  = 2.f * x * y + 2.f * w * z;
  view[5]  = 1.f - 2.f * x * x - 2.f * z * z;
  view[6]  = 2.f * y * z - 2.f * w * x;
  view[7]  = -translation.y();
  view[8]  = 2.f * x * z - 2.f * w * y;
  view[9]  = 2.f * y * z + 2.f * w * x;
  view[10] = 1.f - 2.f * x * x - 2.f * y * y;
  view[11] = -translation.z();
  view[12] = T(0);
  view[13] = T(0);
  view[14] = T(0);
  view[15] = T(1);
  return view;
}

}  // math
