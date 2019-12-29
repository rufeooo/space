#pragma once

#include <utility>
#include <vector>

#include "entity.h"
#include "util.h"

namespace ecs
{
namespace internal
{
template <typename T, typename... TT>
std::pair<Entity, T>*
GetComponentPointer(
    std::tuple<std::vector<std::pair<Entity, TT>>...>& components_storage)
{
  auto& components =
      std::get<std::vector<std::pair<Entity, T>>>(components_storage);
  if (components.empty()) return nullptr;
  return &components[0];
}

template <class Tup>
bool
AdvanceFirst(Tup&& tup)
{
  bool all_zero = true;
  bool done = false;
  // TODO: Implement templated FirstOf.
  util::StaticFor(tup, [&](auto i, auto*& p) {
    if (done) return;
    if (p->first != ENTITY_LIST_END) {
      all_zero = false;
      done = true;
      ++p;
    }
  });
  return all_zero;
}

template <class Tup>
bool
AllEqual(Tup&& tup, Entity& id)
{
  bool all_equal = true;
  id = ENTITY_PLACEHOLDER;
  util::StaticFor(tup, [&](auto i, auto*& p) {
    if (id == ENTITY_PLACEHOLDER)
      id = p->first;
    else if (id != p->first)
      all_equal = false;
  });
  return id != ENTITY_LIST_END && all_equal;
}

template <class Tup>
bool
AnyMax(Tup&& tup)
{
  bool any_zero = false;
  // TODO: Implmement templated FirstOf.
  util::StaticFor(tup, [&](auto i, auto*& p) {
    if (!p) {
      any_zero = true;
      return;
    }
    if (p->first == ENTITY_LIST_END) any_zero = true;
  });
  return any_zero;
}

template <class Tup>
void
AdvanceMin(Tup&& tup)
{
  Entity min = ENTITY_PLACEHOLDER;
  int min_idx = 0;
  util::StaticFor(tup, [&](auto i, auto*& p) {
    if (p->first != ENTITY_LIST_END && p->first < min) {
      min = p->first;
      min_idx = i;
    }
  });
  util::StaticFor(tup, [&](auto i, auto*& p) {
    if (p->first != ENTITY_LIST_END && min_idx == i) {
      ++p;
    }
  });
}

template <class Tup>
void
AdvanceAll(Tup&& tup)
{
  util::StaticFor(tup, [&](auto i, auto*& p) {
    if (p->first != ENTITY_LIST_END) {
      ++p;
    }
  });
}

// Compares entity to component pair.
template <class T>
struct CompareEntity {
  Entity
  AsEntity(const std::pair<Entity, T>& e) const
  {
    return e.first;
  }

  Entity
  AsEntity(Entity e) const
  {
    return e;
  }

  template <typename T1, typename T2>
  bool
  operator()(const T1& t1, const T2& t2) const
  {
    return AsEntity(t1) < AsEntity(t2);
  }
};

}  // namespace internal
}  // namespace ecs
