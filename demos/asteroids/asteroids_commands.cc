#include "asteroids_commands.h"

#include <random>

#include "asteroids.h"
#include "asteroids_state.h"

namespace asteroids {

namespace commands {

void Execute(uint8_t* command_bytes) {
  const asteroids::Command* command =
      asteroids::GetCommand((void*)command_bytes);
  if (command->create_player()) {
    Execute(*command->create_player());
  }
  if (command->create_projectile()) {
    Execute(*command->create_projectile());
  }
  if (command->create_asteroid()) {
    Execute(*command->create_asteroid());
  }
  if (command->acknowledge()) {
  }
}

void Execute(const asteroids::CreatePlayer& create_player) {
  auto& components = GlobalGameState().components;
  components.Assign<PhysicsComponent>(create_player.entity_id());
  components.Assign<PolygonShape>(
      create_player.entity_id(), GlobalEntityGeometry().ship_geometry);
  components.Assign<component::TransformComponent>(
      create_player.entity_id());
  components.Assign<component::RenderingComponent>(
      create_player.entity_id(),
      GlobalOpenGLGameReferences().ship_vao_reference,
      GlobalOpenGLGameReferences().program_reference,
      GlobalEntityGeometry().ship_geometry.size());
  components.Assign<asteroids::InputComponent>(
      create_player.entity_id());
}

void Execute(const asteroids::CreateProjectile& create_projectile) {
  auto& components = GlobalGameState().components;
  component::TransformComponent transform;
  transform.position =
      math::Vec3f(create_projectile.transform().position().x(),
                  create_projectile.transform().position().y(),
                  create_projectile.transform().position().z());
  transform.orientation =
      math::Quatf(create_projectile.transform().orientation().x(),
                  create_projectile.transform().orientation().y(),
                  create_projectile.transform().orientation().z(),
                  create_projectile.transform().orientation().w());
  auto dir = transform.orientation.Up();
  dir.Normalize();
  component::TransformComponent projectile_transform(transform);
  projectile_transform.position += (dir * .08f);
  components.Assign<component::TransformComponent>(
      create_projectile.entity_id(), projectile_transform);
  components.Assign<PhysicsComponent>(
      create_projectile.entity_id(), math::Vec3f(),
      dir * kProjectileSpeed, 0.f, 0.f);
  components.Assign<TTLComponent>(create_projectile.entity_id());
  components.Assign<PolygonShape>(
      create_projectile.entity_id(),
      GlobalEntityGeometry().projectile_geometry);
  components.Assign<component::RenderingComponent>(
      create_projectile.entity_id(),
      GlobalOpenGLGameReferences().projectile_vao_reference,
      GlobalOpenGLGameReferences().program_reference,
      GlobalEntityGeometry().projectile_geometry.size());
  GlobalGameState().projectile_entities.insert(
      create_projectile.entity_id());
}

void Execute(const asteroids::CreateAsteroid& create_asteroid) {
  auto& components = GlobalGameState().components;
  math::Vec3f dir(create_asteroid.direction().x(),
                  create_asteroid.direction().y(),
                  create_asteroid.direction().z());
  dir.Normalize();
  components.Assign<component::TransformComponent>(
      create_asteroid.entity_id());
  auto* transform = components.Get<component::TransformComponent>(
      create_asteroid.entity_id());
  transform->position = math::Vec3f(
      create_asteroid.position().x(),
      create_asteroid.position().y(),
      create_asteroid.position().z());
  transform->orientation.Set(create_asteroid.angle(),
                             math::Vec3f(0.f, 0.f, 1.f));
  components.Assign<PhysicsComponent>(create_asteroid.entity_id());
  auto* physics = components.Get<PhysicsComponent>(
      create_asteroid.entity_id());
  physics->velocity = dir * kShipAcceleration * 50.f;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<>
      disi(0, GlobalEntityGeometry().asteroid_geometry.size() - 1);
  // TODO: random_number also exists in create_asteroid and should come
  // from it when a server exists again.
  int random_number = disi(gen);
  // Store off the random number that was used to create the asteroid.
  // This is useful for server->client communication when the client
  // needs to recreate the asteroid.
  components.Assign<RandomNumberIntChoiceComponent>(
      create_asteroid.entity_id(), (uint8_t)random_number);
  components.Assign<PolygonShape>(
      create_asteroid.entity_id(),
      GlobalEntityGeometry().asteroid_geometry[random_number]);
  components.Assign<component::RenderingComponent>(
      create_asteroid.entity_id(),
      GlobalOpenGLGameReferences()
          .asteroid_vao_references[random_number],
      GlobalOpenGLGameReferences().program_reference,
      GlobalEntityGeometry().asteroid_geometry[random_number].size());
  GlobalGameState().asteroid_entities.insert(
      create_asteroid.entity_id());
}

}

}