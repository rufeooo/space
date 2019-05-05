#pragma once

#include <vector>

#include "entity.h"

namespace ecs {
namespace internal {

template <typename T>
static std::vector<std::pair<Entity, T>> components_;

}  // namespace internal
}  // namespace ecs
