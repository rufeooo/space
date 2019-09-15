#pragma once

#include <array>
#include <cassert>
#include <initializer_list>

namespace math {

// column major matrix
template <typename T, size_t M, size_t N>
class Mat {
 public:
  virtual ~Mat() = default;
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

}  // math
