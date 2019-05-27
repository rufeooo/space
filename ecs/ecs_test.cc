#define CATCH_CONFIG_MAIN  // Make Catch provide main.

#include <iostream>
#include <catch2/catch.hpp>

#include "ecs.h"

struct PositionComponent {
  PositionComponent() = default;  // To insert 0 entities.
  PositionComponent(int x, int y) : x_(x), y_(y) {}

  int x_;
  int y_;
};

struct VelocityComponent {
  VelocityComponent() = default;  // To insert 0 entities.
  VelocityComponent(float dx, float dy) : dx_(dx), dy_(dy) {}

  float dx_;
  float dy_;
};

struct NullComponent {};

TEST_CASE("Sorted Assign / Get", "[ecs]") {
  // TODO: Implement a cleaner way to address this. Since the component
  // list is static it exists between test cases. It'd be nice to be
  // able to clear all component lists I'm just not sure how that
  // would be implemented.
  ecs::Clear<PositionComponent>(); ecs::Clear<VelocityComponent>();
  ecs::Assign<PositionComponent>(1, 10, 15);
  ecs::Assign<VelocityComponent>(1, 1.0f, 3.0f);
  
  REQUIRE(ecs::Get<PositionComponent>(1)->x_ == 10);
  REQUIRE(ecs::Get<PositionComponent>(1)->y_ == 15);
  REQUIRE(ecs::Get<VelocityComponent>(1)->dx_ == 1.0f);
  REQUIRE(ecs::Get<VelocityComponent>(1)->dy_ == 3.0f);

  REQUIRE(ecs::Get<PositionComponent>(15) == nullptr);
  REQUIRE(ecs::Get<NullComponent>(1) == nullptr);
}

TEST_CASE("Unsorted Assign / Get", "[ecs]") {
  ecs::Clear<PositionComponent>(); ecs::Clear<VelocityComponent>();
  ecs::Assign<PositionComponent>(3, 1, 2);
  ecs::Assign<VelocityComponent>(3, 1.0f, 3.0f);
  ecs::Assign<PositionComponent>(1, 5, 7);
  ecs::Assign<VelocityComponent>(1, 5.0f, 15.0f);
  ecs::Assign<PositionComponent>(2, 15, 20);
  ecs::Assign<VelocityComponent>(2, 15.0f, 45.0f);
  ecs::Assign<PositionComponent>(7, 20, 30);
  ecs::Assign<VelocityComponent>(7, 5.0f, 0.0f);
  ecs::Assign<PositionComponent>(4, 3, 4);
  ecs::Assign<VelocityComponent>(4, 1.0f, 3.0f);

  REQUIRE(ecs::Get<VelocityComponent>(2)->dx_ == 15.0f);
  REQUIRE(ecs::Get<PositionComponent>(4)->x_ == 3);
  REQUIRE(ecs::Get<VelocityComponent>(3)->dy_ == 3.0f);
  REQUIRE(ecs::Get<PositionComponent>(1)->y_ == 7);
}

TEST_CASE("Sorted insertion and Enumerate", "[ecs]") {
  struct Foo {};
  struct Bar {};
  struct Baz {};
  // Clear all the component lists for each section.
  ecs::Clear<Foo>(); ecs::Clear<Bar>(); ecs::Clear<Baz>();

  // This Section is to verify the example in the comments of ecs.h
  // works correctly.
  SECTION("Comments section enumeration test.") {
    // Entity 1 has all the components.
    ecs::Assign<Foo>(1);
    ecs::Assign<Bar>(1);
    ecs::Assign<Baz>(1);
    // Entity 2 has component Foo.
    ecs::Assign<Foo>(2);
    // Entity 3 has components Foo and Bar.
    ecs::Assign<Foo>(3);
    ecs::Assign<Bar>(3);
    // Entity 4 has components Foo and Baz.
    ecs::Assign<Foo>(4);
    ecs::Assign<Baz>(4);
    // Verify this runs for all entities.
    std::vector<ecs::Entity> entities;
    ecs::Enumerate<Foo>([&](auto& foo) {
      entities.push_back(foo->first);
    });
    REQUIRE(entities == std::vector<ecs::Entity>({1, 2, 3, 4}));
    entities.clear();
    ecs::Enumerate<Foo, Bar>([&](auto& foo, auto& bar) {
      REQUIRE(foo->first == bar->first);
      entities.push_back(foo->first);
    });
    REQUIRE(entities == std::vector<ecs::Entity>({1, 3}));
    entities.clear();
    ecs::Enumerate<Foo, Baz>([&](auto& foo, auto& baz) {
      REQUIRE(foo->first == baz->first);
      entities.push_back(foo->first);
    });
    REQUIRE(entities == std::vector<ecs::Entity>({1, 4}));
    entities.clear();
    ecs::Enumerate<Foo, Bar, Baz>([&](auto& foo, auto& bar,
                                      auto& baz) {
      REQUIRE(foo->first == bar->first);
      REQUIRE(foo->first == baz->first);
      entities.push_back(foo->first);
    });
    REQUIRE(entities == std::vector<ecs::Entity>({1}));
  }

  SECTION("Large scale test.") {
    for (int i = 1; i < 15000; ++i) ecs::Assign<Foo>(i);
    for (int i = 400; i < 2000; ++i) ecs::Assign<Bar>(i);
    int i = 0;
    ecs::Enumerate<Foo, Bar>([&](auto& foo, auto& bar) {
      REQUIRE(foo->first == bar->first);
      ++i;
    });
    REQUIRE(i == 1600);
  }

  SECTION("Sparse intersection.") {
    std::vector<ecs::Entity> sparse_list(
        {3, 76, 133, 223, 4567, 33456});
    for (int i = 1; i < 50000; ++i) ecs::Assign<Foo>(i);
    for (int i : sparse_list) ecs::Assign<Bar>(i);
    std::vector<ecs::Entity> intersection_list;
    ecs::Enumerate<Foo, Bar>([&](auto& foo, auto& bar) {
      REQUIRE(foo->first == bar->first);
      intersection_list.push_back(foo->first);
    });
    REQUIRE(sparse_list == intersection_list);
  }

  SECTION("Dense intersection.") {
    for (int i = 1; i < 10000; ++i) ecs::Assign<Foo>(i);
    for (int i = 1; i < 10000; ++i) ecs::Assign<Bar>(i);
    for (int i = 1; i < 10000; ++i) ecs::Assign<Baz>(i);
    int i = 1;
    ecs::Enumerate<Foo, Bar, Baz>([&](auto& foo, auto& bar,
                                      auto& baz) {
      REQUIRE(foo->first == bar->first);
      REQUIRE(foo->first == baz->first);
      REQUIRE(foo->first == i);
      ++i;
    });
    REQUIRE(i == 10000);
  }
}

// TODO: Add sorted Assign and then enumeration this will NOT work
// currently but will work when the lists remain sorted.
TEST_CASE("Unsorted insertion and Enumerate", "[ecs]") {
  REQUIRE(true == true);
}

TEST_CASE("Mutating components during Enumerate", "[ecs]") {
  struct Component {
    Component() = default;
    Component(int n) : n_(n) {}; int n_;
  };
  ecs::Clear<Component>();
  for (int i = 0; i < 10; ++i) {
    // Put 0 through 10 in components list.
    ecs::Assign<Component>(i + 1, i);
  }
  // Increment each component so that it is 1 through 11.
  ecs::Enumerate<Component>([](auto& comp) {
    comp->second.n_++;
  });
  // Assert the above assumption is true (1 through 11).
  int i = 1;
  ecs::Enumerate<Component>([&](auto& comp) {
    REQUIRE(comp->second.n_ == i++);
  });
}
