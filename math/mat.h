#pragma once

#include <array>
#include <cassert>
#include <string>
#include <initializer_list>

#include "vec.h"

namespace math {

// column major matrix
template <typename T, size_t M, size_t N>
class Mat {
 public:
  Mat() = default;
  Mat(std::initializer_list<T> l) {
    assert(l.size() == M * N);
    int i = 0;
    for (const auto& v : l) {
      data_[i++] = v;
    }
  }

  T& operator()(size_t m, size_t n) {
    assert(m * n < M * N);
    return data_[n * M + m];
  }

  const T& operator()(size_t m, size_t n) const {
    assert(m * n < M * N);
    return data_[n * M + m];
  }

  T& operator[](size_t i) {
    return data_[i];
  }

  size_t rows() const { return M; }
  size_t cols() const { return N; }

  bool operator==(const Mat& rhs) const {
    return data_ == rhs.data_;
  }

  bool operator!=(const Mat& rhs) const {
    return !(data_ == rhs.data_);
  }

  std::string String() const {
    std::string m = "";
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        m += std::to_string((*this)(i, j));
        if (j != M - 1) m += ",";
      }
      m += "\n";
    }
    return m;
  }

  Mat<T, N, M> Transpose() const {
    Mat<T, N, M> t;
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        t(i, j) = (*this)(j, i);
      }
    }
    return t;
  }

  std::array<T, M * N> data_;
};

// This is fun... It's impossible for a program to compile if the
// multplication is invalid.
template <typename T, size_t M, size_t N, size_t P>
Mat<T, M, P> operator*(const Mat<T, M, N>& lhs,
                       const Mat<T, N, P>& rhs) {
  Mat<T, M, P> r;
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < P; ++j) {
      r(i, j) = T(0);
      for (int k = 0; k < N; ++k) {
        r(i, j) += lhs(i, k) * rhs(k, j);
      }
    }
  }
  return r;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N> operator*(const Mat<T, M, N>& lhs, const T& rhs) {
  Mat<T, M, N> r;
  for (size_t i = 0; i < M * N; ++i) r.data_[i] = lhs.data_[i] * rhs;
  return r;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N> operator*(const T& rhs, const Mat<T, M, N>& lhs) {
  return lhs * rhs;
}


template <typename T, size_t M, size_t N>
Mat<T, M, N> operator/(const Mat<T, M, N>& lhs, const T& rhs) {
  Mat<T, M, N> r;
  for (size_t i = 0; i < M * N; ++i) r.data_[i] = lhs.data_[i] / rhs;
  return r;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N> operator+(const Mat<T, M, N>& lhs, const T& rhs) {
  Mat<T, M, N> r;
  for (size_t i = 0; i < M * N; ++i) r.data_[i] = lhs.data_[i] + rhs;
  return r;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N> operator+(const T& rhs, const Mat<T, M, N>& lhs) {
  return lhs + rhs;
}

template <typename T, size_t M, size_t N>
Mat<T, M, N> operator-(const Mat<T, M, N>& lhs, const T& rhs) {
  Mat<T, M, N> r;
  for (size_t i = 0; i < M * N; ++i) r.data_[i] = lhs.data_[i] - rhs;
  return r;
}

using Mat4i = Mat<int, 4, 4>;
using Mat4u = Mat<uint32_t, 4, 4>;
using Mat4f = Mat<float, 4, 4>;
using Mat4d = Mat<double, 4, 4>;

// Ths matrix vector multiplication is written as if the vector was
// augmented with a 1 (x, y, z, 1). And mutiplied against a matrix
// from the right as a column vector.
//
// This is useful when multiplying transformation matrices, consider
// the following example.
//
// For translation
//
// m = [ 1 0 0 Tx
//       0 1 0 Ty
//       0 0 1 Tz
//       0 0 0 1 ]
//
// v = [ x, y, z ]
//
// m * v = [ x + Tx, y + Ty, z + Tz ]
//
// For scale
//
// m = [ Sx 0  0  0
//       0  Sy 0  0
//       0  0  Sz 0
//       0  0  0  1 ]
//
// v = [ x, y, z ]
//
// m * v = [ x * Sx, y * Sy, z * Sz ]
//
// For some rotation along x by theta
//
// m = [ 1           0           0 0
//       0  cos(theta) -sin(theta) 0
//       0  sin(theta)  cos(theta) 0
//       0           0           0 1 ]
//
// v = [ x, y, z]
//
// m * v = [ x, ycos(theta) - zsin(theta), ysin(theta) + zcos(theta) ]
template <class T>
Vec3f operator*(const Mat<T, 4, 4>& lhs, const Vec3f& rhs) {
  return Vec3f(
    lhs(0, 0) * rhs.x + lhs(0, 1) * rhs.y +
    lhs(0, 2) * rhs.z + lhs(0, 3),
    lhs(1, 0) * rhs.x + lhs(1, 1) * rhs.y +
    lhs(1, 2) * rhs.z + lhs(1, 3),
    lhs(2, 0) * rhs.x + lhs(2, 1) * rhs.y +
    lhs(2, 2) * rhs.z + lhs(2, 3)
  );
}

}  // math
