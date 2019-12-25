#pragma once

#include <vector>

#include "ecs/common.cc"
#include "ecs/internal.h"

#include "components/server_authoritative_component.h"

#include "math/vec.h"

namespace asteroids
{
struct PhysicsComponent {
  PhysicsComponent() = default;
  PhysicsComponent(const math::Vec3f& acceleration, const math::Vec3f& velocity,
                   float acceleration_speed, float max_velocity)
      : acceleration(acceleration),
        velocity(velocity),
        acceleration_speed(acceleration_speed),
        max_velocity(max_velocity)
  {
  }
  math::Vec3f acceleration;
  math::Vec3f velocity;
  // TODO: These are constants in asteroids.h
  float acceleration_speed = 0.00004f;
  float max_velocity = 0.00004f * 3.f * 60.f;
};

struct PolygonShape {
  PolygonShape() = default;
  PolygonShape(const std::vector<math::Vec2f>& points) : points(points) {}
  std::vector<math::Vec2f> points;
};

struct GameStateComponent {
  float seconds_since_last_asteroid_spawn = 0.f;
  uint64_t asteroid_count = 0;
};

struct TTLComponent {
  uint32_t updates_to_live = 266;
};

struct RandomNumberIntChoiceComponent {
  RandomNumberIntChoiceComponent() = default;
  RandomNumberIntChoiceComponent(uint8_t number) : random_number(number) {}
  uint8_t random_number;
};

struct PlayerComponent {
};
struct AsteroidComponent {
};
struct ProjectileComponent {
};

}  // namespace asteroids
