// Public API for ecs system. This header exposes all ECS functions
// a user is meant to use.

#pragma once

#include <algorithm>
#include <tuple>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <vector>

#include "entity.h"
#include "internal.h"
#include "util.h"

namespace ecs {

template <typename... COMPONENTS>
class ComponentStorage {
 public:
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
  //   TODO: This function needs to go or return a copy. This is danger
  //   time if the underlying vector changes beneath the user after the
  //   pointer has been returned.
  template <typename T>
  T* Get(Entity entity) {
    auto& components = std::get<std::vector<std::pair<Entity, T>>>(
        components_);
    auto found = std::lower_bound(
        components.begin(), components.end(),
        entity, internal::CompareEntity<T>());
    if (found != components.end() &&
        (*found).first == entity) return &(*found).second;
    return nullptr;
  }

  template <typename T>
  void Remove(Entity entity) {
    auto& components = std::get<std::vector<std::pair<Entity, T>>>(
        components_);
    auto found = std::lower_bound(
        components.begin(), components.end(),
        entity, internal::CompareEntity<T>());
    if (found != components.end() &&
        (*found).first == entity) {
      components.erase(found);
    }
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
  template <typename T, typename... Args>
  T* Assign(Entity entity, Args&& ...args) {
    auto& components = std::get<std::vector<std::pair<Entity, T>>>(
        components_);
    // Require placeholder at end of component lists because we rely on
    // pointer advancing in vectors and they must have a condition to
    // stop.
    if (components.empty()) {
      // Components must have default constructors.
      components.push_back({ENTITY_LIST_END, T()});
    }
    // Binary search for the entity.
    auto found = std::lower_bound(
        components.begin(), components.end(),
        entity, internal::CompareEntity<T>());
    // If the entity is found overwrite it's component data. Otherwise,
    // insert a new entity in sorted order.
    if ((*found).first == entity) {
      (*found).second = T(std::forward<Args>(args)...);
      return &(*found).second;
    } else {
      auto inserted = components.insert(
          found, {entity, T(std::forward<Args>(args)...)});
      // Store deletion necessary when cleaning up this entity.
      deletion_[entity].push_back([this, entity]() {
        Remove<T>(entity);
      });
      return &inserted->second;
    }
  }

  // This function is used to implement systems. It will intersect all
  // component lists as given by args and execute the function f for
  // every entity that contains all the components in the args list.
  // For example -
  //
  // Assume
  // entity 1 has Components( Foo, Bar, Baz )
  // entity 2 has Components( Foo )
  // entity 3 has Components( Foo, Bar )
  // entity 4 has Components( Foo, Baz )
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
  template <typename... Args, typename F>
  void Enumerate(F&& f) {
    //auto tup = internal::Gather<Args...>(components_);
    auto tup = std::make_tuple(internal::GetComponentPointer<Args>(
        components_)...);
    while (!internal::AnyMax(tup)) {
      Entity entity;
      if (internal::AllEqual(tup, entity)) {
        util::ApplyFromTuple(f, entity, tup);
        internal::AdvanceAll(tup);
        continue;
      }
      internal::AdvanceMin(tup); 
    }
  }

  // Removes all components from the given componet list. For example -
  //
  // Clear<Foo>()
  // Will remove all components of type Foo.
  template <typename T>
  void Clear() {
    auto& components = std::get<std::vector<std::pair<Entity, T>>>(
        components_);
    components.clear(); 
  }

  // Removes all components from every component list given the entity.
  void Delete(ecs::Entity entity) {
    auto found = deletion_.find(entity);
    if (found == deletion_.end()) return;
    for (auto& delete_func : found->second) {
      delete_func();
    }
    deletion_.erase(found);
  }

 private:
  std::tuple<std::vector<std::pair<Entity, COMPONENTS>>...>
      components_;
  std::unordered_map<ecs::Entity, std::vector<std::function<void()>>>
      deletion_;

};


}  // namespace ecs
