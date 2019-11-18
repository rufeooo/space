#pragma once

#include <cstdint>

namespace ecs {

using Entity = uint64_t;

inline constexpr Entity ENTITY_LIST_END = 0xFFFFFFFF;
inline constexpr Entity ENTITY_PLACEHOLDER = ENTITY_LIST_END - 1;
inline constexpr Entity ENTITY_LAST_FREE = ENTITY_LIST_END - 2;

}  // namespace ecs
