#include <iostream>

#include "game/sdl_game.h"
#include "math/vec.h"

struct PositionComponent {
  PositionComponent() = default;
  PositionComponent(const math::Vec2f& position)
    : position(position) {};
  math::Vec2f position;
};

struct PhysicsComponent {
  PhysicsComponent() = default;
  PhysicsComponent(const math::Vec2f& velocity,
                    const math::Vec2f& acceleration)
    : velocity(velocity), acceleration(acceleration) {};
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

class Asteroids : public game::SDLGame {
 public:
  Asteroids() : game::SDLGame(640, 480) {};
  void Update() override {};
};

int main() {
  Asteroids asteroids;
  asteroids.Run();

  return 0;
}
