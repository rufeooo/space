#pragma once

#include <limits>

namespace math
{
template <typename T>
bool
IsNear(T value, T target, T epsilon = std::numeric_limits<T>::epsilon())
{
  return value == target ||
         (target - value < epsilon && target - value > -epsilon);
}

}  // namespace math
