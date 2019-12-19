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
  xy()
  {
    return Vec2<T>(x, y);
  }

  Vec2<T>
  yz()
  {
    return Vec2<T>(y, z);
  }

  Vec2<T>
  xz()
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

using Vec2i = Vec2<int>;
using Vec2u = Vec2<uint32_t>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

using Vec3i = Vec3<int>;
using Vec3u = Vec3<uint32_t>;
using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;

}  // namespace math
