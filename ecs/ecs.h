// Public API for ecs system. This header exposes all ECS functions
// a user is meant to use.

// TODO: Current implementation relies on an entity whose id is 0 be
// put at the end of all component lists. This allows enumeration
// to end when intersecting all component lists. Investigate
// alternative solutions or modify the code to maintain that property
// since it is must now be manually enforced.

#pragma once

#include <tuple>
#include <vector>

#include "components.h"
#include "entity.h"
#include "internal.h"
#include "util.h"

namespace ecs {


// Given an entity retrieves a pointer to the component specified
// by the templated argument. For example -
//
//   Foo* foo = ecs::Get<Foo>(3);
//
// Will -
//
//   return Foo* in the components_ vector as specified by the Entity.
//
// Or -
//
//   return nullptr if the component doesn't exist in the component
//   for that entity id.
//
// TODO: This iterates the component list linearly and
// should do a binary search when the lists are sorted.
template <typename T>
T* Get(Entity entity) {
  for (auto& component : internal::components_<T>) {
    if (component.first == entity) {
      return &component.second;
    }
  }
  return nullptr;
}

// Given a entity id and a constructor list will create a new component
// and forward args to that components constructor. For example -
//
// struct Foo {
//   Foo(int n) : n_(n) {}
//   int n_
// };
//
// ecs::Assign<Foo>(1, 3);
//
// Will create an object of type Foo and append it to the component
// list components_<Foo>.
//
// TODO: These components should be inserted in sorted order.
template <typename T, typename... Args>
void Assign(Entity entity, Args&& ...args) {
  internal::components_<T>.push_back({
    entity,
    T(std::forward<Args>(args)...)
  });
}

// This function is used to implement systems. It will intersect all
// component lists as given by args and execute the function f for
// every entity that contains all the components in the args list.
// For example -
//
// Assume
// entity 1 has Components( Foo, Bar, Baz )
// entity 2 has Components( Foo )
// entity 3 has Components( Foo, Bar)
// entity 4 has Components( Foo, Baz)
//
// Enumerate<Foo>([](auto& foo) {
// });
// Will run for entities (1, 2, 3, 4).
//
// Enumerate<Foo, Bar>([](auto& foo, auto& bar) {
// });
// Will run for entities (1, 3)
//
// Enumerate<Foo, Baz>([](auto& foo, auto& baz) {
// });
// Will run for entities (1, 4)
//
// Enumerate<Foo, Bar, Baz>([](auto& foo, auto& bar, auto& baz) {
// });
// Will run for entity (1)
//
// The type expected in the functor for each component is -
// std::pair<Entity, ComponentType>*.
//
// TODO: If all the entities are matching all the pointers should be
// moved forward after the functor is run. Currently it moves forward
// only min.
// TODO: This intersection relies on a component with entity 0 being
// at the end of each component lists. Mantain that invariant via this
// api or remove it with a better approach.
template <typename... Args, typename F>
void Enumerate(F&& f) {
  auto tup = internal::Gather<Args...>();
  while (!internal::AnyZero(tup)) {
    if (internal::AllEqual(tup)) {
      util::ApplyFromTuple(f, tup);
    }
    internal::AdvanceMin(tup); 
  }
}

}
