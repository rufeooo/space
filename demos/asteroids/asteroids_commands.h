#pragma once

#include <cassert>
#include <functional>

#include "components/common/transform_component.h"
#include "math/vec.h"

namespace asteroids {

namespace commands {

struct CreatePlayer {
  uint64_t entity_id;
  math::Vec3f position;
};

struct CreateProjectile {
  uint64_t entity_id;
  component::TransformComponent transform;
};

struct CreateAsteroid {
  uint64_t entity_id;
  math::Vec3f position;
  math::Vec3f direction;
  float angle;
  int random_number;
};

enum class InputKey {
  NONE  = 0,
  W     = 1,
  A     = 2,
  S     = 4,
  D     = 8,
  SPACE = 16,
};

struct Input {
  uint8_t input_mask;
  uint8_t previous_input_mask;
};

void Execute(CreatePlayer& create_player);
void Execute(CreateProjectile& create_projectile);
void Execute(CreateAsteroid& create_asteroid);
void Execute(Input& input);

}

}
