#pragma once

#include <vector>

#include "math/vec.h"
#include "network/client.h"
#include "network/message_queue.h"

namespace asteroids {

struct InputComponent {
  InputComponent() = default;
  // Set to true when the user request a projectile to be shot.
  bool shoot_projectile = false;
  // State the space bar was last in.
  int space_state;
  // If a was pressed in the past process input calls.
  bool a_pressed = false; 
  // If b was pressed in the past process input calls.
  bool d_pressed = false; 
};

struct PhysicsComponent {
  PhysicsComponent() = default;
  PhysicsComponent(
      const math::Vec3f& acceleration,
      const math::Vec3f& velocity,
      float acceleration_speed,
      float max_velocity) :
    acceleration(acceleration), velocity(velocity),
    acceleration_speed(acceleration_speed),
    max_velocity(max_velocity) {}
  math::Vec3f acceleration;
  math::Vec3f velocity;
  // TODO: These are constants in asteroids.h
  float acceleration_speed = 0.00004f;
  float max_velocity = 0.00004f * 3.f * 60.f;
};

// TODO: Move to components/.
struct PolygonShape {
  PolygonShape() = default;
  PolygonShape(const std::vector<math::Vec2f>& points)
      : points(points) {}
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
  RandomNumberIntChoiceComponent(uint8_t number) :
    random_number(number) {}
  uint8_t random_number;
};

struct ConnectionComponent {
  network::OutgoingMessageQueue outgoing_message_queue;
  network::IncomingMessageQueue incoming_message_queue;
  std::thread network_thread;
  bool is_client = false;
  bool is_server = false;
};

}
