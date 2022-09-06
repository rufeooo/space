#pragma once

#include <cstdint>

// For the given type defines:
//    kMax<type> - The upper bound count for the given type.
//    k<type> - The storage for the type.
//    kUsed<type> - The in-use count of the given type.
// Methods:
//    Use<type>() - Function to request use of a instance of type.
//    Compress<type>(int idx) - Compresses the array starting at idx by moving
//    all elements that occur after idx down one element in the array.
#define DECLARE_ARRAY(type, max_count)             \
  constexpr uint64_t kMax##type = max_count;       \
                                                   \
  static type k##type[max_count];                  \
  static type kZero##type;                         \
                                                   \
  static uint64_t kUsed##type;                     \
                                                   \
  type* Use##type()                                \
  {                                                \
    assert(kUsed##type < kMax##type);              \
    if (kUsed##type >= kMax##type) return nullptr; \
    type* t = &k##type[kUsed##type];               \
    kUsed##type += 1;                              \
    *t = {};                                       \
    return t;                                      \
  }                                                \
                                                   \
  void Compress##type(int idx)                     \
  {                                                \
    if (idx >= kMax##type) return;                 \
    if (idx < 0) return;                           \
    for (int i = idx; i < kUsed##type; ++i) {      \
      k##type[i] = k##type[i + 1];                 \
    }                                              \
    --kUsed##type;                                 \
  }                                                \

#define DECLARE_ND_ARRAY(type, n, max_count)       \
  constexpr uint64_t kMax##type = max_count;       \
  constexpr uint64_t kDim##type = n;               \
  static type k##type[n][max_count];               \
  static type kZero##type;                         \
                                                   \
  static uint64_t kUsed##type[n];                  \
                                                   \
  type* Use##type(uint64_t dim)                    \
  {                                                \
    assert(dim < kDim##type);                      \
    assert(kUsed##type[dim] < kMax##type);         \
    if (kUsed##type[dim] >= kMax##type)            \
      return nullptr;                              \
    type* t = &k##type[dim][kUsed##type[dim]];     \
    kUsed##type[dim] += 1;                         \
    *t = {};                                       \
    return t;                                      \
  }                                                \
  void Compress##type(uint64_t dim, int idx)       \
  {                                                \
    if (idx >= kMax##type) return;                 \
    if (idx < 0) return;                           \
    for (int i = idx; i < kUsed##type[dim]; ++i) { \
      k##type[dim][i] = k##type[dim][i + 1];       \
    }                                              \
    --kUsed##type[dim];                            \
  }                                                \
