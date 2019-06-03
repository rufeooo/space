#include <iostream>

#include "vec.h"

struct PositionComponent {
  PositionComponent() = default;
  PositionComponent(const math::Vec2f& position)
    : position(position) {}
  math::Vec2f position;
};

struct PhysicsComponent {
  PhysicsComponent() = default;
  PositionComponent(const math::Vec2f& velocity,
                    const math::Vec2f& acceleration)
    : velocity(velocity), acceleration(acceleration) {}
  math::Vec2f velocity;
  math::Vec2f acceleration;
};

struct InputComponent {
  InputComponent() = default;
};

struct ShooterComponent {
  ShooterComponent() = default;
};

struct TriangleRenderComponent {
  TriangleRenderComponent() = default;
};

struct CircleRenderComponent {
  CircleRenderComponent() = default;
};

int main() {

  return 0;
}
