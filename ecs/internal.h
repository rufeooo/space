#pragma once

#include "entity.h"
#include "util.h"

namespace ecs {
namespace internal {

template <typename T>
std::pair<Entity, T>* GetComponentPointer() {
  return &components_<T>[0];
}

template <typename... Args>
std::tuple<std::pair<Entity, Args>*...> Gather() {
  return std::make_tuple(GetComponentPointer<Args>()...);
}

template <class Tup>
bool AdvanceFirst(Tup&& tup) {
  bool all_zero = true;
  bool done = false;
  // TODO: Implement templated FirstOf.
  util::StaticFor(tup, [&](auto i, auto*& p) {
    if (done) return;
    if (p->first != 0) {
      all_zero = false;
      done = true;
      ++p;
    }
  });
  return all_zero;
}

template <class Tup>
bool AllEqual(Tup&& tup) {
  bool all_equal = true;
  Entity id = 0xFFFFFFFF;
  util::StaticFor(tup, [&](auto i, auto*& p) {
    if (id == 0xFFFFFFFF) id = p->first;
    else if (id != p->first) all_equal = false;
  });
  return id != 0 && all_equal;
}

template <class Tup>
bool AnyZero(Tup&& tup) {
  bool any_zero = false;
  // TODO: Implmement templated FirstOf. 
  util::StaticFor(tup, [&](auto i, auto*& p) {
    if (p->first == 0) any_zero = true;
  });
  return any_zero;
}

template <class Tup>
void AdvanceMin(Tup&& tup) {
  Entity min = 0xFFFFFFFF;
  int min_idx = 0;
  util::StaticFor(tup, [&](auto i, auto*& p) {
    if (p->first != 0 && p->first < min) {
      min = p->first;
      min_idx = i;
    }
  });
  util::StaticFor(tup, [&](auto i, auto*& p) {
    if (p->first != 0 && min_idx == i) {
      ++p;
    }
  });
}

}
}
