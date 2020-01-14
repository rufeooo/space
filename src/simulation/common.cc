#pragma once

#include <cstdint>

// For the given type defines -
//    kMax<type> - The upper bound count for the given type.
//    k<type> - The storage for the type.
//    kUsed<type> - The in-use count of the given type.
//    Use<type>() - Function to request use of a instance of type.
//    Release<type>() - Function to return an instance of type.
#define DECLARE_GAME_TYPE(type, max_count)      \
  constexpr uint64_t kMax##type = max_count;    \
                                                \
  static type k##type[max_count];               \
                                                \
  static uint64_t kUsed##type;                  \
                                                \
  type* Use##type()                             \
  {                                             \
    if (kUsed##type >= kMax##type) return NULL; \
    type* t = &k##type[kUsed##type];            \
    kUsed##type += 1;                           \
    return t;                                   \
  }                                             \
                                                \
  void Release##type(uint64_t id)               \
  {                                             \
    uint64_t used = kUsed##type;                \
    if (!used) return;                          \
    if (id >= used) return;                     \
    used -= 1;                                  \
    kUsed##type = used;                         \
    if (id == used) return;                     \
    k##type[id] = k##type[used];                \
  }

// For the given type defines -
//    kMax<type> - The upper bound count for the given type.
//    k<type> - The storage for the type.
//    kRead<type> - Read index
//    kWrite<type> - Write index
#define DECLARE_GAME_QUEUE(type, max_count)               \
                                                          \
  constexpr uint64_t kMax##type = max_count;              \
                                                          \
  static type k##type[max_count];                         \
                                                          \
  static uint64_t kRead##type;                            \
  static uint64_t kWrite##type;                           \
                                                          \
  type Pop##type()                                        \
  {                                                       \
    if (kWrite##type - kRead##type == 0) return type{};   \
    type ret = k##type[kRead##type];                      \
    kRead##type += 1;                                     \
    kRead##type %= kMax##type;                            \
    return ret;                                           \
  }                                                       \
                                                          \
  void Push##type(type val)                               \
  {                                                       \
    uint64_t next_slot = (kWrite##type + 1) % kMax##type; \
    if (next_slot == kRead##type) return;                 \
    k##type[kWrite##type] = val;                          \
    kWrite##type = next_slot;                             \
  }

