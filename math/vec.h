#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <string>

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

  bool operator==(const Vec<T, N>& rhs) const {
    for (size_t i = 0; i < rhs.size(); ++i) {
      if (data_[i] != rhs.data_[i]) return false;
    }
    return true;
  }

  bool operator!=(const Vec<T, N>& rhs) const {
    return !(*this == rhs);
  }

  T& operator[](std::size_t i) { return data_[i]; }

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

  const std::array<T, N>& Data() const { return data_; }

  std::string String() const {
    std::string str;
    for (size_t i = 0; i < data_.size(); ++i) {
      str += std::to_string(data_[i]);
      if (i != data_.size() - 1) str += " ";
    }
    return str;
  }


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

  Vec2<T> operator+(const Vec2<T>& rhs) const {
    Vec2<T> t = *this;
    t += rhs;
    return t;
  }

  Vec2<T> operator-(const Vec2<T>& rhs) const {
    Vec2<T> t = *this;
    t -= rhs;
    return t;
  }

  Vec2<T> operator*(const T& rhs) const {
    Vec2<T> t = *this;
    t *= rhs;
    return t;
  }

  Vec2<T> operator/(const T& rhs) const {
    Vec2<T> t = *this;
    t /= rhs;
    return t;
  }

  T& x() { return this->data_[0]; };
  T& y() { return this->data_[1]; };

  T x() const { return this->data_[0]; };
  T y() const { return this->data_[1]; };
};

template <class T>
T Cross(const Vec2<T>& lhs, const Vec2<T>& rhs) {
  return lhs.x() * rhs.y() - lhs.y() * rhs.x();
}

template <typename T>
class Vec3 : public Vec<T, 3> {
 public:
  Vec3() {
    this->data_[0] = T(0);
    this->data_[1] = T(0);
    this->data_[2] = T(0);
  }

  Vec3(const T& x, const T& y, const T& z) {
    this->data_[0] = x;
    this->data_[1] = y;
    this->data_[2] = z;
  }

  Vec3(const Vec<T, 3>& v) {
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

  Vec3<T> operator+(const Vec3<T>& rhs) const {
    Vec3<T> t = *this;
    t += rhs;
    return t;
  }

  Vec3<T> operator-(const Vec3<T>& rhs) const {
    Vec3<T> t = *this;
    t -= rhs;
    return t;
  }

  Vec3<T> operator*(const T& rhs) const {
    Vec3<T> t = *this;
    t *= rhs;
    return t;
  }

  Vec3<T> operator/(const T& rhs) const {
    Vec3<T> t = *this;
    t /= rhs;
    return t;
  }

  T& x() { return this->data_[0]; };
  T& y() { return this->data_[1]; };
  T& z() { return this->data_[2]; };

  T x() const { return this->data_[0]; };
  T y() const { return this->data_[1]; };
  T z() const { return this->data_[2]; };

  math::Vec2<T> xy() const {
    return math::Vec2<T>(x(), y());
  }
};

template <typename T>
class Vec4 : public Vec<T, 4> {
 public:
  Vec4() {
    this->data_[0] = T(0);
    this->data_[1] = T(0);
    this->data_[2] = T(0);
    this->data_[3] = T(0);
  }

  Vec4(const T& x, const T& y, const T& z, const T& w) {
    this->data_[0] = x;
    this->data_[1] = y;
    this->data_[2] = z;
    this->data_[3] = w;
  }

  Vec4(const Vec<T, 4>& v) {
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

  Vec4<T> operator+(const Vec4<T>& rhs) const {
    Vec4<T> t = *this;
    t += rhs;
    return t;
  }

  Vec4<T> operator-(const Vec4<T>& rhs) const {
    Vec4<T> t = *this;
    t -= rhs;
    return t;
  }

  Vec4<T> operator*(const T& rhs) const {
    Vec4<T> t = *this;
    t *= rhs;
    return t;
  }

  Vec4<T> operator/(const T& rhs) const {
    Vec4<T> t = *this;
    t /= rhs;
    return t;
  }

  T& x() { return this->data_[0]; };
  T& y() { return this->data_[1]; };
  T& z() { return this->data_[2]; };
  T& w() { return this->data_[3]; };

  T x() const { return this->data_[0]; };
  T y() const { return this->data_[1]; };
  T z() const { return this->data_[2]; };
  T w() const { return this->data_[3]; };
};

using Vec2i = Vec2<int>;
using Vec2u = Vec2<uint32_t>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

using Vec3i = Vec3<int>;
using Vec3u = Vec3<uint32_t>;
using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;

using Vec4i = Vec4<int>;
using Vec4u = Vec4<uint32_t>;
using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;

}
