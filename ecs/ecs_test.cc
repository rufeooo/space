#define CATCH_CONFIG_MAIN  // Make Catch provide main.

#include <iostream>
#include <catch2/catch.hpp>

#include "ecs.h"

struct PositionComponent {
  PositionComponent() = default;
  PositionComponent(int x, int y) : x_(x), y_(y) {}

  int x_;
  int y_;
};

struct VelocityComponent {
  VelocityComponent() = default;
  VelocityComponent(float dx, float dy) : dx_(dx), dy_(dy) {}

  float dx_;
  float dy_;
};

struct NullComponent {};

TEST_CASE("Sorted Assign / Get", "[ecs]") {
  ecs::ComponentStorage<PositionComponent,
                         VelocityComponent,
                         NullComponent> storage;
  // TODO: Implement a cleaner way to address this. Since the component
  // list is static it exists between test cases. It'd be nice to be
  // able to clear all component lists I'm just not sure how that
  // would be implemented.
  storage.Assign<PositionComponent>(1, 10, 15);
  storage.Assign<VelocityComponent>(1, 1.0f, 3.0f);
  
  REQUIRE(storage.Get<PositionComponent>(1)->x_ == 10);
  REQUIRE(storage.Get<PositionComponent>(1)->y_ == 15);
  REQUIRE(storage.Get<VelocityComponent>(1)->dx_ == 1.0f);
  REQUIRE(storage.Get<VelocityComponent>(1)->dy_ == 3.0f);

  REQUIRE(storage.Get<PositionComponent>(15) == nullptr);
  //REQUIRE(storage.Get<NullComponent>(1) == nullptr);
}
TEST_CASE("Unsorted Assign / Get", "[ecs]") {
  ecs::ComponentStorage<PositionComponent, VelocityComponent> storage;
  storage.Assign<PositionComponent>(3, 1, 2);
  storage.Assign<VelocityComponent>(3, 1.0f, 3.0f);
  storage.Assign<PositionComponent>(1, 5, 7);
  storage.Assign<VelocityComponent>(1, 5.0f, 15.0f);
  storage.Assign<PositionComponent>(2, 15, 20);
  storage.Assign<VelocityComponent>(2, 15.0f, 45.0f);
  storage.Assign<PositionComponent>(7, 20, 30);
  storage.Assign<VelocityComponent>(7, 5.0f, 0.0f);
  storage.Assign<PositionComponent>(4, 3, 4);
  storage.Assign<VelocityComponent>(4, 1.0f, 3.0f);

  REQUIRE(storage.Get<VelocityComponent>(2)->dx_ == 15.0f);
  REQUIRE(storage.Get<PositionComponent>(4)->x_ == 3);
  REQUIRE(storage.Get<VelocityComponent>(3)->dy_ == 3.0f);
  REQUIRE(storage.Get<PositionComponent>(1)->y_ == 7);
}

TEST_CASE("Removed", "[ecs]") {
  ecs::ComponentStorage<PositionComponent, VelocityComponent> storage;
  storage.Assign<PositionComponent>(3, 1, 2);
  storage.Assign<VelocityComponent>(3, 1.0f, 3.0f);
  storage.Assign<PositionComponent>(1, 5, 7);
  storage.Assign<VelocityComponent>(1, 5.0f, 15.0f);
  storage.Assign<PositionComponent>(2, 15, 20);
  storage.Assign<VelocityComponent>(2, 15.0f, 45.0f);
  storage.Assign<PositionComponent>(7, 20, 30);
  storage.Assign<VelocityComponent>(7, 5.0f, 0.0f);
  storage.Assign<PositionComponent>(4, 3, 4);
  storage.Assign<VelocityComponent>(4, 1.0f, 3.0f);
  REQUIRE(storage.Get<VelocityComponent>(2) != nullptr);
  storage.Remove<VelocityComponent>(2);
  REQUIRE(storage.Get<VelocityComponent>(2) == nullptr);
  int i = 0;
  storage.Enumerate<PositionComponent, VelocityComponent>(
      [&](ecs::Entity ent, PositionComponent&, VelocityComponent&) {
    REQUIRE(ent != 2);
    ++i;
  }); 
  REQUIRE(i == 4);
}

TEST_CASE("Sorted insertion and Enumerate", "[ecs]") {
  struct Foo {};
  struct Bar {};
  struct Baz {};

  // This Section is to verify the example in the comments of ecs.h
  // works correctly.
  SECTION("Comments section enumeration test.") {
    ecs::ComponentStorage<Foo, Bar, Baz> storage;
    // Entity 1 has all the components.
    storage.Assign<Foo>(1);
    storage.Assign<Bar>(1);
    storage.Assign<Baz>(1);
    // Entity 2 has component Foo.
    storage.Assign<Foo>(2);
    // Entity 3 has components Foo and Bar.
    storage.Assign<Foo>(3);
    storage.Assign<Bar>(3);
    // Entity 4 has components Foo and Baz.
    storage.Assign<Foo>(4);
    storage.Assign<Baz>(4);
    // Verify this runs for all entities.
    std::vector<ecs::Entity> entities;
    storage.Enumerate<Foo>([&](ecs::Entity e, Foo& /*foo*/) {
      entities.push_back(e);
    });
    REQUIRE(entities == std::vector<ecs::Entity>({1, 2, 3, 4}));
    entities.clear();
    storage.Enumerate<Foo, Bar>([&](
        ecs::Entity e, Foo& /*foo*/, Bar& /*bar*/) {
      entities.push_back(e);
    });
    REQUIRE(entities == std::vector<ecs::Entity>({1, 3}));
    entities.clear();
    storage.Enumerate<Foo, Baz>([&](ecs::Entity e, Foo& foo, Baz& baz) {
      entities.push_back(e);
    });
    REQUIRE(entities == std::vector<ecs::Entity>({1, 4}));
    entities.clear();
    storage.Enumerate<Foo, Bar, Baz>([&](
        ecs::Entity e, Foo& foo, Bar& bar, Baz& baz) {
      entities.push_back(e);
    });
    REQUIRE(entities == std::vector<ecs::Entity>({1}));
  }

  SECTION("Large scale test.") {
    ecs::ComponentStorage<Foo, Bar, Baz> storage;
    for (int i = 1; i < 15000; ++i) storage.Assign<Foo>(i);
    for (int i = 400; i < 2000; ++i) storage.Assign<Bar>(i);
    int i = 0;
    storage.Enumerate<Foo, Bar>([&](ecs::Entity e, Foo& foo, Bar& bar) {
      ++i;
    });
    REQUIRE(i == 1600);
  }

  SECTION("Sparse intersection.") {
    ecs::ComponentStorage<Foo, Bar, Baz> storage;
    std::vector<ecs::Entity> sparse_list(
        {3, 76, 133, 223, 4567, 33456});
    for (int i = 1; i < 50000; ++i) storage.Assign<Foo>(i);
    for (int i : sparse_list) storage.Assign<Bar>(i);
    std::vector<ecs::Entity> intersection_list;
    storage.Enumerate<Foo, Bar>([&](ecs::Entity e, Foo& foo, Bar& bar) {
      intersection_list.push_back(e);
    });
    REQUIRE(sparse_list == intersection_list);
  }

  SECTION("Dense intersection.") {
    ecs::ComponentStorage<Foo, Bar, Baz> storage;
    for (int i = 1; i < 10000; ++i) storage.Assign<Foo>(i);
    for (int i = 1; i < 10000; ++i) storage.Assign<Bar>(i);
    for (int i = 1; i < 10000; ++i) storage.Assign<Baz>(i);
    int i = 1;
    storage.Enumerate<Foo, Bar, Baz>([&](
        ecs::Entity e, Foo& foo, Bar& bar, Baz& baz) {
      REQUIRE(e == i);
      ++i;
    });
    REQUIRE(i == 10000);
  }
}
// TODO: Add sorted Assign and then enumeration this will NOT work
// currently but will work when the lists remain sorted.
TEST_CASE("Unsorted insertion and Enumerate", "[ecs]") {
  ecs::ComponentStorage<PositionComponent, VelocityComponent> storage;
  storage.Assign<PositionComponent>(3, 1, 2);
  storage.Assign<PositionComponent>(1, 5, 7);
  storage.Assign<VelocityComponent>(1, 5.0f, 15.0f);
  storage.Assign<PositionComponent>(2, 15, 20);
  storage.Assign<VelocityComponent>(2, 15.0f, 45.0f);
  storage.Assign<PositionComponent>(7, 20, 30);
  storage.Assign<VelocityComponent>(7, 5.0f, 0.0f);
  storage.Assign<PositionComponent>(4, 3, 4);
  storage.Assign<VelocityComponent>(4, 1.0f, 3.0f);

  std::vector<ecs::Entity> entities;
  storage.Enumerate<PositionComponent, VelocityComponent>([&]
      (ecs::Entity e,
       PositionComponent& /*position*/,
       VelocityComponent& /*velocity*/) {
    entities.push_back(e);
  });
  REQUIRE(entities ==
      std::vector<ecs::Entity>({1, 2, 4, 7}));
}

TEST_CASE("Mutating components during Enumerate", "[ecs]") {
  struct Component {
    Component() = default;
    Component(int n) : n_(n) {}; int n_;
  };
  ecs::ComponentStorage<Component> storage;
  for (int i = 0; i < 10; ++i) {
    // Put 0 through 10 in components list.
    storage.Assign<Component>(i + 1, i);
  }
  // Increment each component so that it is 1 through 11.
  storage.Enumerate<Component>([](ecs::Entity /*e*/, Component& comp) {
    comp.n_++;
  });
  // Assert the above assumption is true (1 through 11).
  int i = 1;
  storage.Enumerate<Component>([&](ecs::Entity /*e*/, Component& comp) {
    REQUIRE(comp.n_ == i++);
  });
}
