#define CATCH_CONFIG_MAIN  // Make Catch provide main.

#include <iostream>
#include <catch2/catch.hpp>

#include "ecs.h"
#include "system.h"

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
    std::cout << "Entity: " << entity << std::endl;
    std::cout << position.x_ << "," << position.y_ << std::endl;
    std::cout << velocity.dx_ << "," << velocity.dy_ << std::endl;
    position.x_ *= 2;
    position.y_ *= 2;
    velocity.dx_ *= 2.0f;
    velocity.dy_ *= 2.0f;
  }
};

TEST_CASE("Simple System Test", "[system]") {
  ecs::Clear<PositionComponent>(); ecs::Clear<VelocityComponent>();
  ecs::Assign<PositionComponent>(1, 10, 15);
  ecs::Assign<VelocityComponent>(1, 1.0f, 3.0f);
  PhysicsSystem physics_system;
  physics_system.Run();
  physics_system.Run();
}
