#define CATCH_CONFIG_MAIN  // Make Catch provide main.

#include <iostream>
#include "gtest/gtest.h"

#include "ecs.h"
#include "mock_system.h"

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

class PhysicsSystem
    : public ecs::System<PositionComponent, VelocityComponent> {
 public:
  void OnEntity(
      ecs::Entity entity,
      PositionComponent& position,
      VelocityComponent& velocity) override {
    position.x_ *= 2;
    position.y_ *= 2;
    velocity.dx_ *= 2.0f;
    velocity.dy_ *= 2.0f;
  }
};

TEST(SystemTest, HappyPathTest) {
  ecs::Clear<PositionComponent>(); ecs::Clear<VelocityComponent>();
  // Runs for entity 1.
  ecs::Assign<PositionComponent>(1, 10, 15);
  ecs::Assign<VelocityComponent>(1, 1.0f, 3.0f);
  // Does not run for entity 2.
  ecs::Assign<PositionComponent>(2, 10, 15);
  // Runs for entity 3.
  ecs::Assign<PositionComponent>(3, 5, 5);
  ecs::Assign<VelocityComponent>(3, 5.0f, 5.0f);
  PhysicsSystem physics_system;
  physics_system.Run();
  auto* position_one = ecs::Get<PositionComponent>(1);
  auto* velocity_one = ecs::Get<VelocityComponent>(1);
  EXPECT_EQ(position_one->x_, 20);
  EXPECT_EQ(position_one->y_, 30);
  EXPECT_EQ(velocity_one->dx_, 2.0f);
  EXPECT_EQ(velocity_one->dy_, 6.0f);
  auto* position_three = ecs::Get<PositionComponent>(3);
  auto* velocity_three = ecs::Get<VelocityComponent>(3);
  EXPECT_EQ(position_three->x_, 10);
  EXPECT_EQ(position_three->y_, 10);
  EXPECT_EQ(velocity_three->dx_, 10.0f);
  EXPECT_EQ(velocity_three->dy_, 10.0f);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
