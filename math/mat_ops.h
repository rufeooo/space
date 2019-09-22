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
  auto f = quat.Forward();
  auto u = quat.Up();
  auto l = quat.Left();
  return Mat<T, 4, 4> {
     l.x(),  l.y(),  l.z(), T(0),
     u.x(),  u.y(),  u.z(), T(0),
    -f.x(), -f.y(), -f.z(), T(0),
      T(0),   T(0),   T(0), T(1)
  };
}

template <class T>
Mat<T, 4, 4> CreateViewMatrix(const Vec3<T>& translation,
                              const Quat<T>& quat) {
  auto f = quat.Forward();
  auto u = quat.Up();
  auto l = quat.Left();
  return Mat<T, 4, 4> {
     l.x(),  l.y(),  l.z(), translation.x(), 
     u.x(),  u.y(),  u.z(), translation.y(),
    -f.x(), -f.y(), -f.z(), translation.z(),
      T(0),   T(0),   T(0), T(1)
  };
}

}  // math
