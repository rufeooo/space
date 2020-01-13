#pragma once

#include <cassert>

struct Transform {
  math::Vec3f position;
  math::Vec3f scale = math::Vec3f(1.f, 1.f, 1.f);
  math::Quatf orientation;
};

// For the given type defines -
//    kMaxSize<type> - Max allowed for the given game type.
//    kCount<type> - The number of the given game type.
//    k<type> - The storage for the type.
//    New<type>() - Function to create a new of the given type.
#define DECLARE_GAME_TYPE(type, max_count)     \
  constexpr int kMaxSize##type = max_count;    \
                                               \
  static type k##type[max_count];              \
                                               \
  static int kCount##type;                     \
                                               \
  type* New##type() {                          \
    assert(kCount##type + 1 < kMaxSize##type); \
    type* t = &k##type[kCount##type++];        \
    return t;                                  \
  }                                            \
                                               \
  void Delete##type(int id) {                  \
    for (int i = id; i < kCount##type; ++i) {  \
      k##type[i] = k##type[i + 1];             \
    }                                          \
    --kCount##type;                            \
  }                                            \
