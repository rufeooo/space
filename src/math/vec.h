#pragma once

#include <cmath>
#include <cstdint>

namespace math
{
template <typename T>
struct Vec2 {
  Vec2()
  {
    // Worth default init?
    x = T(0);
    y = T(0);
  }

  Vec2(const T& x, const T& y) : x(x), y(y) {}

  void
  operator+=(const Vec2<T>& rhs)
  {
    x += rhs.x;
    y += rhs.y;
  }

  Vec2<T>
  operator+(const Vec2<T>& rhs) const
  {
    Vec2<T> t = *this;
    t += rhs;
    return t;
  }

  void
  operator-=(const T& rhs)
  {
    x -= rhs;
    y -= rhs;
  }

  Vec2<T>
  operator-(const T& rhs) const
  {
    Vec2<T> t = *this;
    t -= rhs;
    return t;
  }

  Vec2<T>
  operator-() const
  {
    return Vec2<T>() - *this;
  }

  void
  operator-=(const Vec2<T>& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
  }

  Vec2<T>
  operator-(const Vec2<T>& rhs) const
  {
    Vec2<T> t = *this;
    t -= rhs;
    return t;
  }

  void
  operator*=(const T& rhs)
  {
    x *= rhs;
    y *= rhs;
  }

  Vec2<T> operator*(const T& rhs) const
  {
    Vec2<T> t = *this;
    t *= rhs;
    return t;
  }

  void
  operator/=(const T& rhs)
  {
    x /= rhs;
    y /= rhs;
  }

  Vec2<T>
  operator/(const T& rhs) const
  {
    Vec2<T> t = *this;
    t /= rhs;
    return t;
  }

  void
  operator/=(const Vec2<T>& rhs)
  {
    x /= rhs.x;
    y /= rhs.y;
  }

  Vec2<T>
  operator/(const Vec2<T>& rhs) const
  {
    Vec2<T> t = *this;
    t /= rhs;
    return t;
  }

  bool
  operator==(const Vec2<T>& rhs) const
  {
    return x == rhs.x && y == rhs.y;
  }

  bool
  operator!=(const Vec2<T>& rhs) const
  {
    return !(*this == rhs);
  }

  T x;
  T y;
};

template <class T>
T
Dot(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

template <typename T>
float
LengthSquared(const Vec2<T>& v)
{
  return Dot(v, v);
}

template <typename T>
float
Length(const Vec2<T>& v)
{
  return std::sqrt(Dot(v, v));
}

template <class T>
T
Cross(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
  return lhs.x() * rhs.y() - lhs.y() * rhs.x();
}

template <class T>
Vec2<T>
Normalize(const Vec2<T>& v)
{
  return v / Length(v);
}

template <typename T>
struct Vec3 {
  Vec3()
  {
    // Worth default init?
    x = T(0);
    y = T(0);
    z = T(0);
  }

  Vec3(const T& x, const T& y, const T& z) : x(x), y(y), z(z) {}

  Vec3(const Vec2<T>& v2) : x(v2.x), y(v2.y), z(0.f) {}

  void
  operator+=(const Vec3<T>& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
  }

  Vec3<T>
  operator+(const Vec3<T>& rhs) const
  {
    Vec3<T> t = *this;
    t += rhs;
    return t;
  }

  void
  operator-=(const Vec3<T>& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
  }

  Vec3<T>
  operator-(const Vec3<T>& rhs) const
  {
    Vec3<T> t = *this;
    t -= rhs;
    return t;
  }

  Vec3<T>
  operator-() const
  {
    return Vec3<T>() - *this;
  }

  void
  operator*=(const T& rhs)
  {
    x *= rhs;
    y *= rhs;
    z *= rhs;
  }

  Vec3<T> operator*(const T& rhs) const
  {
    Vec3<T> t = *this;
    t *= rhs;
    return t;
  }

  void
  operator/=(const T& rhs)
  {
    x /= rhs;
    y /= rhs;
    z /= rhs;
  }

  Vec3<T>
  operator/(const T& rhs) const
  {
    Vec3<T> t = *this;
    t /= rhs;
    return t;
  }

  void
  operator/=(const Vec3<T>& rhs)
  {
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
  }

  Vec3<T>
  operator/(const Vec3<T>& rhs) const
  {
    Vec3<T> t = *this;
    t /= rhs;
    return t;
  }

  bool
  operator==(const Vec3<T>& rhs) const
  {
    return x == rhs.x && y == rhs.y && z == rhs.z;
  }

  bool
  operator!=(const Vec3<T>& rhs) const
  {
    return !(*this == rhs);
  }

  Vec2<T>
  xy() const
  {
    return Vec2<T>(x, y);
  }

  Vec2<T>
  yz() const
  {
    return Vec2<T>(y, z);
  }

  Vec2<T>
  xz() const
  {
    return Vec2<T>(x, z);
  }

  T x;
  T y;
  T z;
};

template <class T>
T
Dot(const Vec3<T>& lhs, const Vec3<T>& rhs)
{
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template <typename T>
float
LengthSquared(const Vec3<T>& v)
{
  return Dot(v, v);
}

template <typename T>
float
Length(const Vec3<T>& v)
{
  return std::sqrt(Dot(v, v));
}

template <class T>
Vec3<T>
Normalize(const Vec3<T>& v)
{
  return v / Length(v);
}

//       i j k
// lhs = a b c
// rhs = x y z
//
// lhs cross rhs =
// bz - cy, cx - az, ay - bx

template <class T>
Vec3<T>
Cross(const Vec3<T>& lhs, const Vec3<T>& rhs)
{
  return Vec3<T>(lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z,
                 lhs.x * rhs.y - lhs.y * rhs.x);
}

template <typename T>
struct Vec4 {
  Vec4()
  {
    x = T(0);
    y = T(0);
    z = T(0);
    w = T(1);  // Worthwhile default for homogeneous cords and color.
  }

  Vec4(const T& x, const T& y, const T& z, const T& w) : x(x), y(y), z(z), w(w)
  {
  }

  void
  operator+=(const Vec4<T>& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
  }

  Vec4<T>
  operator+(const Vec4<T>& rhs) const
  {
    Vec4<T> t = *this;
    t += rhs;
    return t;
  }

  void
  operator-=(const Vec4<T>& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
  }

  Vec4<T>
  operator-(const Vec4<T>& rhs) const
  {
    Vec4<T> t = *this;
    t -= rhs;
    return t;
  }

  Vec4<T>
  operator-() const
  {
    return Vec4<T>() - *this;
  }

  void
  operator-=(T rhs)
  {
    x -= rhs;
    y -= rhs;
    z -= rhs;
    w -= rhs;
  }

  Vec4<T>
  operator-(T rhs) const
  {
    Vec4<T> t = *this;
    t -= rhs;
    return t;
  }

  void
  operator*=(const T& rhs)
  {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    w *= rhs;
  }

  Vec4<T> operator*(const T& rhs) const
  {
    Vec4<T> t = *this;
    t *= rhs;
    return t;
  }

  void
  operator/=(const T& rhs)
  {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    w /= rhs;
  }

  Vec4<T>
  operator/(const T& rhs) const
  {
    Vec4<T> t = *this;
    t /= rhs;
    return t;
  }

  void
  operator/=(const Vec4<T>& rhs)
  {
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    w /= rhs.w;
  }

  Vec4<T>
  operator/(const Vec4<T>& rhs) const
  {
    Vec4<T> t = *this;
    t /= rhs;
    return t;
  }

  bool
  operator==(const Vec4<T>& rhs) const
  {
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
  }

  bool
  operator!=(const Vec4<T>& rhs) const
  {
    return !(*this == rhs);
  }

  Vec3<T>
  xyz() const
  {
    return Vec3<T>(x, y, z);
  }

  T x;
  T y;
  T z;
  T w;
};

template <class T>
T
Dot(const Vec4<T>& lhs, const Vec4<T>& rhs)
{
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

template <typename T>
float
LengthSquared(const Vec4<T>& v)
{
  return Dot(v, v);
}

template <typename T>
float
Length(const Vec4<T>& v)
{
  return std::sqrt(Dot(v, v));
}

template <class T>
Vec4<T>
Normalize(const Vec4<T>& v)
{
  return v / Length(v);
}

}  // namespace math

using v2i = math::Vec2<int>;
using v2f = math::Vec2<float>;

using v3i = math::Vec3<int>;

using v3f = math::Vec3<float>;

using v4f = math::Vec4<float>;

void
Printv3f(v3f v)
{
  printf("%.5f,%.5f,%.5f\n", v.x, v.y, v.z);
}

void
Printv3i(v3i v)
{
  printf("%i,%i,%i\n", v.x, v.y, v.z);
}

void
Printv2f(v2f v)
{
  printf("%.5f,%.5f\n", v.x, v.y);
}
