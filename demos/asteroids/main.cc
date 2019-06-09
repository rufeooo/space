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
  void Update() override {}
  void Render() override {
    // Clear the screen to white.
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, SDL_ALPHA_OPAQUE);
    //Drawing the lines we want. (-1, 0), (0, 1), (1, 0)
    SDL_RenderDrawLine(renderer_, 0, 480, 320, 0);
    SDL_RenderDrawLine(renderer_, 640, 480, 320, 0);
    SDL_RenderDrawLine(renderer_, 0, 480, 640, 480);
    // Present.
    SDL_RenderPresent(renderer_);
  }
 private:
  ecs::Entity player_ = 1;
};

int main() {
  Asteroids asteroids;
  asteroids.Run();

  return 0;
}
