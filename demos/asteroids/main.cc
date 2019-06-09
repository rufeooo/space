#include <iostream>

#include "ecs/ecs.h"
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
  void Initialize() override {
    SDLGame::Initialize();
    ecs::Assign<PositionComponent>(player_, math::Vec2f(0.0f, 0.0f));
    ecs::Assign<PhysicsComponent>(player_);
    ecs::Assign<InputComponent>(player_);
    ecs::Assign<ShooterComponent>(player_);
    ecs::Assign<TriangleRenderComponent>(player_);
  }
  void Update() override {};
 private:
  ecs::Entity player_ = 1;
};

int main() {
  Asteroids asteroids;
  asteroids.Run();

  return 0;
}
