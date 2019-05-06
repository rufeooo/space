#define CATCH_CONFIG_MAIN  // Make Catch provide main.

#include <iostream>
#include <catch2/catch.hpp>

#include "ecs.h"

struct PositionComponent {
  PositionComponent(int x, int y) : x_(x), y_(y) {}

  int x_;
  int y_;
};

struct VelocityComponent {
  VelocityComponent(float dx, float dy) : dx_(dx), dy_(dy) {}

  float dx_;
  float dy_;
};

struct NullComponent {};

TEST_CASE("Assign / Get", "[ecs]") {
  ecs::Assign<PositionComponent>(1, 10, 15);
  ecs::Assign<VelocityComponent>(1, 1.0f, 3.0f);
  
  REQUIRE(ecs::Get<PositionComponent>(1)->x_ == 10);
  REQUIRE(ecs::Get<PositionComponent>(1)->y_ == 15);
  REQUIRE(ecs::Get<VelocityComponent>(1)->dx_ == 1.0f);
  REQUIRE(ecs::Get<VelocityComponent>(1)->dy_ == 3.0f);

  REQUIRE(ecs::Get<PositionComponent>(0) == nullptr);
  REQUIRE(ecs::Get<NullComponent>(1) == nullptr);
}

TEST_CASE("Enumerate", "[ecs]") {
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
    // TODO: Don't manually add 0 entities to component lists.
    ecs::Assign<Foo>(0);
    ecs::Assign<Bar>(0);
    ecs::Assign<Baz>(0);
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
    //TODO: Don't manually add 0 entities to component lists.
    ecs::Assign<Foo>(0);
    ecs::Assign<Bar>(0);
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
    //TODO: Don't manually add 0 entities to component lists.
    ecs::Assign<Foo>(0);
    ecs::Assign<Bar>(0);
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
    //TODO: Don't manually add 0 entities to component lists.
    ecs::Assign<Foo>(0);
    ecs::Assign<Bar>(0);
    ecs::Assign<Baz>(0);
    ecs::Enumerate<Foo, Bar, Baz>([&](auto& foo, auto& bar,
                                      auto& baz) {
      REQUIRE(foo->first == bar->first);
      REQUIRE(foo->first == baz->first);
      REQUIRE(foo->first == i);
      ++i;
    });
  }
}
