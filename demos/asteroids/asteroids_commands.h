#pragma once

#include <cassert>
#include <functional>

#include "components/common/transform_component.h"
#include "math/vec.h"

namespace asteroids {

namespace commands {

enum Event {
  CREATE_PLAYER = 0,
  CREATE_ASTEROID = 1,
  CREATE_PROJECTILE = 2,
  PLAYER_INPUT = 3,
  DELETE_ENTITY = 4,
  PLAYER_ID_MUTATION = 5,
};

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
  uint64_t entity_id;
  uint8_t input_mask;
  uint8_t previous_input_mask;
};

struct DeleteEntity {
  uint64_t entity_id;
};

struct PlayerIdMutation {
  uint64_t entity_id;
};

void Execute(CreatePlayer& create_player);
void Execute(CreateProjectile& create_projectile);
void Execute(CreateAsteroid& create_asteroid);
void Execute(Input& input);
void Execute(DeleteEntity& delete_entity);
void Execute(PlayerIdMutation& change_player_id);

}

}
