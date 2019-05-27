#pragma once

namespace ecs {

using Entity = uint64_t;

constexpr Entity ENTITY_LIST_END = 0xFFFFFFFF;
constexpr Entity ENTITY_PLACEHOLDER = ENTITY_LIST_END - 1;

}  // namespace ecs
