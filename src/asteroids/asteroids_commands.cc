#include "asteroids_commands.h"

#include <random>

#include "asteroids.h"
#include "asteroids_state.h"
#include "ecs.cc"

namespace asteroids
{
namespace commands
{
void
Execute(CreatePlayer& create_player)
{
  auto& components = GlobalGameState().components;
  components.Assign<PhysicsComponent>(create_player.entity_id);
  components.Assign<PolygonShape>(create_player.entity_id,
                                  GlobalEntityGeometry().ship_geometry);
  components.Assign<TransformComponent>(create_player.entity_id);
  components.Assign<RenderingComponent>(
      create_player.entity_id, GlobalOpenGLGameReferences().ship_vao_reference,
      GlobalOpenGLGameReferences().program_reference,
      GlobalEntityGeometry().ship_geometry.size());
  components.Assign<InputComponent>(create_player.entity_id);
  components.Assign<PlayerComponent>(create_player.entity_id);
  auto* auth =
      components.Assign<ServerAuthoritativeComponent>(create_player.entity_id);
  auth->bitmask = TRANSFORM | PHYSICS;
}

void
Execute(CreateProjectile& create_projectile)
{
  auto& components = GlobalGameState().components;
  auto dir = math::Normalize(create_projectile.transform.orientation.Up());
  TransformComponent projectile_transform(create_projectile.transform);
  projectile_transform.position += (dir * .08f);
  components.Assign<TransformComponent>(create_projectile.entity_id,
                                        projectile_transform);
  components.Assign<PhysicsComponent>(create_projectile.entity_id,
                                      math::Vec3f(), dir * kProjectileSpeed,
                                      0.f, 0.f);
  components.Assign<TTLComponent>(create_projectile.entity_id);
  components.Assign<PolygonShape>(create_projectile.entity_id,
                                  GlobalEntityGeometry().projectile_geometry);
  components.Assign<RenderingComponent>(
      create_projectile.entity_id,
      GlobalOpenGLGameReferences().projectile_vao_reference,
      GlobalOpenGLGameReferences().program_reference,
      GlobalEntityGeometry().projectile_geometry.size());
  components.Assign<ProjectileComponent>(create_projectile.entity_id);
  auto* auth = components.Assign<ServerAuthoritativeComponent>(
      create_projectile.entity_id);
  auth->bitmask = TRANSFORM;
}

void
Execute(CreateAsteroid& create_asteroid)
{
  auto& components = GlobalGameState().components;
  math::Vec3f dir = math::Normalize(create_asteroid.direction);
  components.Assign<TransformComponent>(create_asteroid.entity_id);
  auto* transform =
      components.Get<TransformComponent>(create_asteroid.entity_id);
  transform->position = create_asteroid.position;
  transform->orientation.Set(create_asteroid.angle, math::Vec3f(0.f, 0.f, 1.f));
  components.Assign<PhysicsComponent>(create_asteroid.entity_id);
  auto* physics = components.Get<PhysicsComponent>(create_asteroid.entity_id);
  physics->velocity = dir * kShipAcceleration * 50.f;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> disi(
      1, GlobalEntityGeometry().asteroid_geometry.size());
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
  components.Assign<RandomNumberIntChoiceComponent>(create_asteroid.entity_id,
                                                    (uint8_t)random_number);
  // Use random_number - 1 so we an assume 0 is unset and can
  // differentiate between providing a random number vs not.
  components.Assign<PolygonShape>(
      create_asteroid.entity_id,
      GlobalEntityGeometry().asteroid_geometry[random_number - 1]);
  components.Assign<RenderingComponent>(
      create_asteroid.entity_id,
      GlobalOpenGLGameReferences().asteroid_vao_references[random_number - 1],
      GlobalOpenGLGameReferences().program_reference,
      GlobalEntityGeometry().asteroid_geometry[random_number - 1].size());
  components.Assign<AsteroidComponent>(create_asteroid.entity_id);
  components.Assign<ServerAuthoritativeComponent>(create_asteroid.entity_id);
  auto* auth = components.Assign<ServerAuthoritativeComponent>(
      create_asteroid.entity_id);
  // Only need transform on asteroids. Physics unneeded.
  auth->bitmask = TRANSFORM;
}

void
Execute(Input& input)
{
  auto& components = GlobalGameState().components;
  auto* cinput = components.Get<InputComponent>(input.entity_id);
  if (!cinput) return;
  cinput->input_mask = input.input_mask;
  cinput->previous_input_mask = input.previous_input_mask;
}

void
Execute(DeleteEntity& delete_entity)
{
  GlobalGameState().components.Delete(delete_entity.entity_id);
}

void
Execute(PlayerIdMutation& change_player_id)
{
  GlobalGameState().player_id = change_player_id.entity_id;
}

void
Execute(UpdateTransform& update_transform)
{
  auto& components = GlobalGameState().components;
  auto* transform =
      components.Get<TransformComponent>(update_transform.entity_id);
  if (!transform) return;
  *transform = update_transform.transform;
}

void
Execute(UpdatePhysics& update_physics)
{
  auto& components = GlobalGameState().components;
  auto* physics = components.Get<PhysicsComponent>(update_physics.entity_id);
  if (!physics) return;
  *physics = update_physics.physics;
}

}  // namespace commands

}  // namespace asteroids
