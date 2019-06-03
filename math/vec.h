#pragma once

#include <array>
#include <cassert>
#include <cmath>

namespace math {

template <typename T, size_t N>
class Vec {
 public:
  virtual ~Vec() = default;

  void operator+=(const Vec& rhs) {
    for (size_t i = 0; i < data_.size(); ++i) {
      data_[i] += rhs.data_[i];
    }
  }

  Vec<T, N> operator+(const Vec& rhs) const {
    Vec<T, N> t = *this;
    t += rhs;
    return t;
  }

  void operator-=(const Vec& rhs) {
    for (size_t i = 0; i < data_.size(); ++i) {
      data_[i] -= rhs.data_[i];
    }
  }

  Vec<T, N> operator-(const Vec& rhs) const {
    Vec<T, N> t = *this;
    t -= rhs;
    return t;
  }

  void operator*=(const T& rhs) {
    for (size_t i = 0; i < data_.size(); ++i) {
      data_[i] *= rhs;
    }
  }

  Vec<T, N> operator*(const T& rhs) const {
    Vec<T, N> t = *this;
    t *= rhs;
    return t;
  }

  void operator/=(const T& rhs) {
    for (size_t i = 0; i < data_.size(); ++i) {
      data_[i] /= rhs;
    }
  }

  Vec<T, N> operator/(const T& rhs) const {
    Vec<T, N> t = *this;
    t /= rhs;
    return t;
  }

  T Dot(const Vec& rhs) const {
    T result = T(0);
    for (size_t i = 0; i < rhs.size(); ++i) {
      result += (data_[i] * rhs.data_[i]);
    }
    return result;
  }

  void Normalize() {
    float l = Length(*this);
    *this /= l;
  }

  const std::array<T, N> Data() const { return data_; }

  std::size_t size() const { return data_.size(); }

 protected:
  std::array<T, N> data_;
};

template <typename T, size_t N>
T Dot(const Vec<T, N>& lhs, const Vec<T, N>& rhs) {
  T result = T(0);
  for (size_t i = 0; i < rhs.size(); ++i) {
    result += (lhs.Data()[i] * rhs.Data()[i]);
  }
  return result;
}

template <typename T, size_t N>
float LengthSquared(const Vec<T, N>& v) {
  return Dot(v, v);
}

template <typename T, size_t N>
float Length(const Vec<T, N>& v) {
  return std::sqrt(Dot(v, v));
}

template <typename T>
class Vec2 : public Vec<T, 2> {
 public:
  Vec2() {
    this->data_[0] = T(0);
    this->data_[1] = T(0);
  }

  Vec2(const T& x, const T& y) {
    this->data_[0] = x;
    this->data_[1] = y;
  }

  Vec2(const Vec<T, 2>& v) {
    this->data_ = v.Data();
  }

  // Following overloads required so Vec2<T> can be properly capture
  // with auto.
  //
  // Example -
  //
  // math::Vec2<int> a;
  // math::Vec2<int> b;
  // auto c = a + b; c = a - b; etc.
  //
  // c will be captured as type Vec<int, 2> instead of Vec2<int> 
  // without these overloads thereby ruining the point of these
  // convenience classes.

  Vec2<T> operator+(const Vec2<T>& rhs) {
    Vec2<T> t = *this;
    t += rhs;
    return t;
  }

  Vec2<T> operator-(const Vec2<T>& rhs) {
    Vec2<T> t = *this;
    t -= rhs;
    return t;
  }

  Vec2<T> operator*(const T& rhs) {
    Vec2<T> t = *this;
    t *= rhs;
    return t;
  }

  Vec2<T> operator/(const T& rhs) {
    Vec2<T> t = *this;
    t /= rhs;
    return t;
  }

  T& x() { return this->data_[0]; };
  T& y() { return this->data_[1]; };
};

using Vec2i = Vec2<int>;
using Vec2u = Vec2<uint32_t>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

}
