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

TEST_CASE("Assigning Components", "[ecs]") {
  ecs::Assign<PositionComponent>(1, 10, 15);
  ecs::Assign<VelocityComponent>(1, 1.0f, 3.0f);
  
  REQUIRE(ecs::Get<PositionComponent>(1)->x_ == 10);
  REQUIRE(ecs::Get<PositionComponent>(1)->y_ == 15);
  REQUIRE(ecs::Get<VelocityComponent>(1)->dx_ == 1.0f);
  REQUIRE(ecs::Get<VelocityComponent>(1)->dy_ == 3.0f);

  REQUIRE(ecs::Get<PositionComponent>(0) == nullptr);
  REQUIRE(ecs::Get<NullComponent>(1) == nullptr);
}
