#pragma once

#include <limits>

namespace math
{

template <typename T>
T
Max(const T& a, const T& b)
{
  return a > b ? a : b;
}

template <typename T>
T
Min(const T& a, const T& b)
{
  return a < b ? a : b;
}

template <typename T>
bool
IsNear(T value, T target, T epsilon = std::numeric_limits<T>::epsilon())
{
  return value == target ||
         (target - value < epsilon && target - value > -epsilon);
}

}  // namespace math
