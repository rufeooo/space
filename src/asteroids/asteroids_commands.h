#pragma once

#include <cassert>
#include <functional>

#include "asteroids_components.h"
#include "math/vec.h"

namespace asteroids
{
namespace commands
{
enum AuthorityBitmask {
  NONE = 0x00,
  TRANSFORM = 0x01,
  PHYSICS = 0x02,
};

enum Event {
  CREATE_PLAYER = 0,
  CREATE_ASTEROID = 1,
  CREATE_PROJECTILE = 2,
  PLAYER_INPUT = 3,
  DELETE_ENTITY = 4,
  PLAYER_ID_MUTATION = 5,

  // These two events are not explicitly handled in the event
  // buffer but instead used to inform the server that the client
  // has created or deleted entities the server is responsible for updating.
  CLIENT_CREATE_AUTHORITATIVE = 6,
  CLIENT_DELETE_AUTHORITATIVE = 7,

  UPDATE_TRANSFORM = 8,
  UPDATE_PHYSICS = 9,

  SERVER_PLAYER_JOIN = 10,
};

struct CreatePlayer {
  uint64_t entity_id;
  math::Vec3f position;
};

struct CreateProjectile {
  uint64_t entity_id;
  TransformComponent transform;
};

struct CreateAsteroid {
  uint64_t entity_id;
  math::Vec3f position;
  math::Vec3f direction;
  float angle;
  int random_number;
};

enum class InputKey {
  NONE = 0,
  W = 1,
  A = 2,
  S = 4,
  D = 8,
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

// TODO: Where should these go?
struct ClientCreateAuthoritative {
  uint64_t entity_id;
};

struct ClientDeleteAuthoritative {
  uint64_t entity_id;
};

struct UpdateTransform {
  uint64_t entity_id;
  TransformComponent transform;
};

struct UpdatePhysics {
  uint64_t entity_id;
  PhysicsComponent physics;
};

struct ServerPlayerJoin {
  int client_id;
};

void Execute(CreatePlayer& create_player);
void Execute(CreateProjectile& create_projectile);
void Execute(CreateAsteroid& create_asteroid);
void Execute(Input& input);
void Execute(DeleteEntity& delete_entity);
void Execute(PlayerIdMutation& change_player_id);
void Execute(UpdateTransform& update_transform);
void Execute(UpdatePhysics& update_physics);
void Execute(ServerPlayerJoin& server_player_join);

}  // namespace commands

}  // namespace asteroids
