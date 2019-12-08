#include "asteroids_commands.h"

#include <random>

#include "asteroids.h"
#include "asteroids_state.h"
#include "components/common/input_component.h"
#include "components/network/server_authoritative_component.h"
#include "integration/entity_replication/entity_replication_server.h"

namespace asteroids {

namespace commands {

void Execute(CreatePlayer& create_player) {
  auto& components = GlobalGameState().components;
  components.Assign<PhysicsComponent>(create_player.entity_id);
  components.Assign<PolygonShape>(
      create_player.entity_id, GlobalEntityGeometry().ship_geometry);
  components.Assign<component::TransformComponent>(
      create_player.entity_id);
  components.Assign<component::RenderingComponent>(
      create_player.entity_id,
      GlobalOpenGLGameReferences().ship_vao_reference,
      GlobalOpenGLGameReferences().program_reference,
      GlobalEntityGeometry().ship_geometry.size());
  components.Assign<component::InputComponent>(
      create_player.entity_id);
  components.Assign<component::ServerAuthoritativeComponent>(
      create_player.entity_id);
}

void Execute(CreateProjectile& create_projectile) {
  auto& components = GlobalGameState().components;
  auto dir = math::Normalize(
      create_projectile.transform.orientation.Up());
  component::TransformComponent projectile_transform(
      create_projectile.transform);
  projectile_transform.position += (dir * .08f);
  components.Assign<component::TransformComponent>(
      create_projectile.entity_id, projectile_transform);
  components.Assign<PhysicsComponent>(
      create_projectile.entity_id, math::Vec3f(),
      dir * kProjectileSpeed, 0.f, 0.f);
  components.Assign<TTLComponent>(create_projectile.entity_id);
  components.Assign<PolygonShape>(
      create_projectile.entity_id,
      GlobalEntityGeometry().projectile_geometry);
  components.Assign<component::RenderingComponent>(
      create_projectile.entity_id,
      GlobalOpenGLGameReferences().projectile_vao_reference,
      GlobalOpenGLGameReferences().program_reference,
      GlobalEntityGeometry().projectile_geometry.size());
  components.Assign<ProjectileComponent>(create_projectile.entity_id);
  components.Assign<component::ServerAuthoritativeComponent>(
      create_projectile.entity_id);
}

void Execute(CreateAsteroid& create_asteroid) {
  auto& components = GlobalGameState().components;
  math::Vec3f dir = math::Normalize(create_asteroid.direction);
  components.Assign<component::TransformComponent>(
      create_asteroid.entity_id);
  auto* transform = components.Get<component::TransformComponent>(
      create_asteroid.entity_id);
  transform->position = create_asteroid.position;
  transform->orientation.Set(create_asteroid.angle,
                             math::Vec3f(0.f, 0.f, 1.f));
  components.Assign<PhysicsComponent>(create_asteroid.entity_id);
  auto* physics = components.Get<PhysicsComponent>(
      create_asteroid.entity_id);
  physics->velocity = dir * kShipAcceleration * 50.f;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<>
      disi(1, GlobalEntityGeometry().asteroid_geometry.size());
  // TODO: random_number also exists in create_asteroid and should come
  // from it when a server exists again.
  int random_number = 0;
  if (create_asteroid.random_number != 0) {
    random_number = create_asteroid.random_number;
  } else {
    random_number = disi(gen);
  }
  create_asteroid.random_number = random_number;
  // Store off the random number that was used to create the asteroid.
  // This is useful for server->client communication when the client
  // needs to recreate the asteroid.
  components.Assign<RandomNumberIntChoiceComponent>(
      create_asteroid.entity_id, (uint8_t)random_number);
  // Use random_number - 1 so we an assume 0 is unset and can
  // differentiate between providing a random number vs not.
  components.Assign<PolygonShape>(
      create_asteroid.entity_id,
      GlobalEntityGeometry().asteroid_geometry[random_number - 1]);
  components.Assign<component::RenderingComponent>(
      create_asteroid.entity_id,
      GlobalOpenGLGameReferences()
          .asteroid_vao_references[random_number - 1],
      GlobalOpenGLGameReferences().program_reference,
      GlobalEntityGeometry()
          .asteroid_geometry[random_number - 1].size());
  components.Assign<AsteroidComponent>(create_asteroid.entity_id);
  CreateAsteroid create_command(create_asteroid);
  create_command.random_number = random_number;
  components.Assign<component::ServerAuthoritativeComponent>(
      create_asteroid.entity_id);
}

void Execute(Input& input) {
  auto& components = GlobalGameState().components;
  auto* cinput = components.Get<component::InputComponent>(input.entity_id);
  if (!cinput) return;
  cinput->input_mask = input.input_mask;
  cinput->previous_input_mask = input.previous_input_mask;
}

void Execute(DeleteEntity& delete_entity) {
  GlobalGameState().components.Delete(delete_entity.entity_id);
}

void Execute(PlayerIdMutation& change_player_id) {
  GlobalGameState().player_id = change_player_id.entity_id;
}

}

}
