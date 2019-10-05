#pragma once

#include "mat.h"
#include "vec.h"
#include "quat.h"

namespace math {

#define ONE_DEG_IN_RAD (2.0 * PI) / 360.0 // 0.017444444

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
Mat<T, 4, 4> CreateScaleMatrix(const Vec3<T>& scale) {
  Mat<T, 4, 4> m = CreateIdentityMatrix<T, 4>();
  m(0, 0) = scale.x();
  m(1, 1) = scale.y();
  m(2, 2) = scale.z();
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
  auto w = quat.x(); // x Corresponds to w in the versor
  auto x = quat.y(); // y Corresponds to y in the versor
  auto y = quat.z(); // z Etc.
  auto z = quat.w(); // w Etc.
  view[0]  = 1.f - 2.f * y * y - 2.f * z * z;
  view[1]  = 2.f * x * y - 2.f * w * z;
  view[2]  = 2.f * x * z + 2 * w * y;
  view[3]  = T(0);
  view[4]  = 2.f * x * y + 2.f * w * z;
  view[5]  = 1.f - 2.f * x * x - 2.f * z * z;
  view[6]  = 2.f * y * z - 2.f * w * x;
  view[7]  = T(0);
  view[8]  = 2.f * x * z - 2.f * w * y;
  view[9]  = 2.f * y * z + 2.f * w * x;
  view[10] = 1.f - 2.f * x * x - 2.f * y * y;
  view[11] = T(0);
  view[12] = -translation.x();
  view[13] = -translation.y();
  view[14] = -translation.z();
  view[15] = T(1);
  return view;
}

template <class T>
Mat<T, 4, 4> CreatePerspectiveMatrix(int width, int height) {
  float near = 0.1f; // clipping plane
  float far = 100.0f; // clipping plane
  float fov = 67.0f * ONE_DEG_IN_RAD; // convert 67 degrees to radians
  float aspect = (float)width / (float)height; // aspect ratio
  // matrix components
  float range = tan(fov * 0.5f) * near;
  float Sx = (2.0f * near) / (range * aspect + range * aspect);
  float Sy = near / range;
  float Sz = -(far + near) / (far - near);
  float Pz = -(2.0f * far * near) / (far - near);
  return Mat<T, 4, 4> {
      Sx, 0.0f, 0.0f, 0.0f,
      0.0f, Sy, 0.0f, 0.0f,
      0.0f, 0.0f, Sz, -1.0f,
      0.0f, 0.0f, Pz, 0.0f
  };
}

}  // math
