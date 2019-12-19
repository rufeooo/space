#pragma once

#include <iostream>

#include "mat.h"
#include "quat.h"
#include "vec.h"

namespace math
{
#define ONE_DEG_IN_RAD (2.0 * PI) / 360.0  // 0.017444444

template <class T, size_t N>
Mat<T, N, N>
CreateIdentityMatrix()
{
  Mat<T, N, N> m;
  // Matrices don't 0 initializate. Maybe they should.
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      m(i, j) = i == j ? static_cast<T>(1) : static_cast<T>(0);
    }
  }
  return m;
}

template <class T>
Mat<T, 4, 4>
CreateTranslationMatrix(const Vec3<T>& translation)
{
  Mat<T, 4, 4> m = CreateIdentityMatrix<T, 4>();
  m[12] = translation.x;
  m[13] = translation.y;
  m[14] = translation.z;
  return m;
}

template <class T>
Mat<T, 4, 4>
CreateScaleMatrix(const Vec3<T>& scale)
{
  Mat<T, 4, 4> m = CreateIdentityMatrix<T, 4>();
  m(0, 0) = scale.x;
  m(1, 1) = scale.y;
  m(2, 2) = scale.z;
  return m;
}

template <class T>
Mat<T, 4, 4>
CreateRotationMatrix(const Quat<T>& quat)
{
  Mat<T, 4, 4> rotation;
  rotation[0] = 1.f - 2.f * quat.y * quat.y - 2.f * quat.z * quat.z;
  rotation[1] = 2.f * quat.x * quat.y - 2.f * quat.w * quat.z;
  rotation[2] = 2.f * quat.x * quat.z + 2 * quat.w * quat.y;
  rotation[3] = T(0);
  rotation[4] = 2.f * quat.x * quat.y + 2.f * quat.w * quat.z;
  rotation[5] = 1.f - 2.f * quat.x * quat.x - 2.f * quat.z * quat.z;
  rotation[6] = 2.f * quat.y * quat.z - 2.f * quat.w * quat.x;
  rotation[7] = T(0);
  rotation[8] = 2.f * quat.x * quat.z - 2.f * quat.w * quat.y;
  rotation[9] = 2.f * quat.y * quat.z + 2.f * quat.w * quat.x;
  rotation[10] = 1.f - 2.f * quat.x * quat.x - 2.f * quat.y * quat.y;
  rotation[11] = T(0);
  rotation[12] = T(0);
  rotation[13] = T(0);
  rotation[14] = T(0);
  rotation[15] = T(1);
  return rotation;
}

template <class T>
Mat<T, 4, 4>
CreateViewMatrix(const Vec3<T>& translation, const Quat<T>& quat)
{
  Mat<T, 4, 4> view;
  view[0] = 1.f - 2.f * quat.y * quat.y - 2.f * quat.z * quat.z;
  view[1] = 2.f * quat.x * quat.y - 2.f * quat.w * quat.z;
  view[2] = 2.f * quat.x * quat.z + 2 * quat.w * quat.y;
  view[3] = T(0);
  view[4] = 2.f * quat.x * quat.y + 2.f * quat.w * quat.z;
  view[5] = 1.f - 2.f * quat.x * quat.x - 2.f * quat.z * quat.z;
  view[6] = 2.f * quat.y * quat.z - 2.f * quat.w * quat.x;
  view[7] = T(0);
  view[8] = 2.f * quat.x * quat.z - 2.f * quat.w * quat.y;
  view[9] = 2.f * quat.y * quat.z + 2.f * quat.w * quat.x;
  view[10] = 1.f - 2.f * quat.x * quat.x - 2.f * quat.y * quat.y;
  view[11] = T(0);
  view[12] = -translation.x;
  view[13] = -translation.y;
  view[14] = -translation.z;
  view[15] = T(1);
  return view;
}

template <class T>
Mat<T, 4, 4>
CreatePerspectiveMatrix(int width, int height)
{
  float near_clip = 0.1f;                       // clipping plane
  float far_clip = 100.0f;                      // clipping plane
  float fov = 67.0f * ONE_DEG_IN_RAD;           // convert 67 degrees to radians
  float aspect = (float)width / (float)height;  // aspect ratio
  // matrix components
  float range = tan(fov * 0.5f) * near_clip;
  float Sx = (2.0f * near_clip) / (range * aspect + range * aspect);
  float Sy = near_clip / range;
  float Sz = -(far_clip + near_clip) / (far_clip - near_clip);
  float Pz = -(2.0f * far_clip * near_clip) / (far_clip - near_clip);
  return Mat<T, 4, 4>{Sx,   0.0f, 0.0f, 0.0f,  0.0f, Sy,   0.0f, 0.0f,
                      0.0f, 0.0f, Sz,   -1.0f, 0.0f, 0.0f, Pz,   0.0f};
}

template <class T>
Mat<T, 4, 4>
CreateOrthographicMatrix(float right, float left, float top, float bottom,
                         float far, float near)
{
  // Goal with this matrix is to scale a point, in likely screen space relative
  // to the cameras to GL space or the unit cube.
  //
  // To do that use the diagonal of this matrix to to scale the point
  // down to a unit cube and then the translation components to move
  // the point into cube space.
  float w = right - left;
  w = w == 0.f ? 1.f : w;
  float h = top - bottom;
  h = h == 0.f ? 1.f : h;
  float d = far - near;
  d = d == 0.f ? 1.f : d;
  return Mat<T, 4, 4>{2.f / w,
                      0.f,
                      0.f,
                      0.f,
                      0.f,
                      2.f / h,
                      0.f,
                      0.f,
                      0.f,
                      0.f,
                      -2.f / d,
                      0.f,
                      -(right + left) / w,
                      -(top + bottom) / h,
                      -(far + near) / d,
                      1.f};
}

}  // namespace math
