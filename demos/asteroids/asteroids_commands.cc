#include "asteroids_commands.h"

#include <random>

#include "asteroids.h"
#include "asteroids_state.h"
#include "components/network/server_authoritative_component.h"

namespace asteroids {

namespace commands {

void Execute(uint8_t* command_bytes) {
  asteroids::Command* command =
      asteroids::GetMutableCommand((void*)command_bytes);
  if (command->create_player()) {
    Execute(*command->mutable_create_player(), true);
  }
  if (command->delete_player()) {
    Execute(*command->mutable_delete_player(), true);
  }
  if (command->create_projectile()) {
    Execute(*command->mutable_create_projectile(), true);
  }
  if (command->create_asteroid()) {
    Execute(*command->mutable_create_asteroid(), true);
  }
  if (command->update_transform()) {
    Execute(*command->mutable_update_transform(), true);
  }
  if (command->acknowledge()) {
  }
}

void Execute(asteroids::CreatePlayer& create_player, bool is_remote) {
  auto& singleton_components = GlobalGameState().singleton_components;
  ConnectionComponent* connection =
    singleton_components.Get<ConnectionComponent>();
  uint64_t original_entity = create_player.entity_id();
  if (is_remote && connection->is_server) {
    create_player.mutate_entity_id(GenerateFreeEntity());
  }
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
  std::cout << "Created player: " << create_player.entity_id() << std::endl;
  if (connection->is_client && !is_remote) {
    flatbuffers::FlatBufferBuilder fbb;
    auto create_command =
        asteroids::CreateCommand(fbb, 0, &create_player);
    fbb.Finish(create_command);
    connection->outgoing_message_queue.Enqueue(fbb.Release());
  } else if (connection->is_server) {
    flatbuffers::FlatBufferBuilder fbb;
    auto delete_player = asteroids::DeletePlayer(original_entity);
    auto delete_and_create_command =
        asteroids::CreateCommand(fbb, 0, &create_player,
                                 &delete_player);
    fbb.Finish(delete_and_create_command);
    connection->outgoing_message_queue.Enqueue(fbb.Release());
  }
}

void Execute(asteroids::DeletePlayer& delete_player, bool is_remote) {
  auto& components = GlobalGameState().components;
  components.Remove<PhysicsComponent>(delete_player.entity_id());
  components.Remove<PolygonShape>(delete_player.entity_id());
  components.Remove<component::TransformComponent>(
      delete_player.entity_id());
  components.Remove<component::RenderingComponent>(
      delete_player.entity_id());
  components.Remove<asteroids::InputComponent>(
      delete_player.entity_id());
  std::cout << "Delete player: " << delete_player.entity_id() << std::endl;
}


void Execute(asteroids::CreateProjectile& create_projectile,
             bool is_remote) {
  std::cout << "Spawn projectile: " << create_projectile.entity_id() << std::endl;
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
  GlobalGameState().projectile_entities.push_back(
      {create_projectile.entity_id(), create_projectile});
}

void Execute(asteroids::CreateAsteroid& create_asteroid,
             bool is_remote) {
  auto& components = GlobalGameState().components;
  math::Vec3f dir(create_asteroid.direction().x(),
                  create_asteroid.direction().y(),
                  create_asteroid.direction().z());
  dir.Normalize();
  std::cout << "CreateAsteroid: " << create_asteroid.entity_id() << std::endl;
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
      disi(1, GlobalEntityGeometry().asteroid_geometry.size());
  // TODO: random_number also exists in create_asteroid and should come
  // from it when a server exists again.
  int random_number = 0;
  if (create_asteroid.random_number() != 0) {
    random_number = create_asteroid.random_number();
  } else {
    random_number = disi(gen);
  }
  create_asteroid.mutate_random_number(random_number);
  // Store off the random number that was used to create the asteroid.
  // This is useful for server->client communication when the client
  // needs to recreate the asteroid.
  components.Assign<RandomNumberIntChoiceComponent>(
      create_asteroid.entity_id(), (uint8_t)random_number);
  // Use random_number - 1 so we an assume 0 is unset and can
  // differentiate between providing a random number vs not.
  components.Assign<PolygonShape>(
      create_asteroid.entity_id(),
      GlobalEntityGeometry().asteroid_geometry[random_number - 1]);
  components.Assign<component::RenderingComponent>(
      create_asteroid.entity_id(),
      GlobalOpenGLGameReferences()
          .asteroid_vao_references[random_number - 1],
      GlobalOpenGLGameReferences().program_reference,
      GlobalEntityGeometry()
          .asteroid_geometry[random_number - 1].size());
  asteroids::CreateAsteroid create_command(create_asteroid);
  create_command.mutate_random_number(random_number);
  GlobalGameState().asteroid_entities.push_back(
      {create_asteroid.entity_id(), create_command});
  auto& singleton_components = GlobalGameState().singleton_components;
  ConnectionComponent* connection =
    singleton_components.Get<ConnectionComponent>();
  if (connection->is_server || connection->is_client) {
    components.Assign<component::ServerAuthoratativeComponent<
        component::TransformComponent>>(create_asteroid.entity_id());
  }
  if (connection->is_server) {
    flatbuffers::FlatBufferBuilder fbb;
    auto create_command =
        asteroids::CreateCommand(fbb, 0, 0, 0, 0, &create_asteroid);
    fbb.Finish(create_command);
    connection->outgoing_message_queue.Enqueue(fbb.Release());
  }
}

void Execute(asteroids::UpdateTransform& update_transform,
             bool is_remote) {
  component::TransformComponent transform;
  transform.position =
      math::Vec3f(update_transform.transform().position().x(),
                  update_transform.transform().position().y(),
                  update_transform.transform().position().z());
  transform.orientation =
      math::Quatf(update_transform.transform().orientation().x(),
                  update_transform.transform().orientation().y(),
                  update_transform.transform().orientation().z(),
                  update_transform.transform().orientation().w());
  transform.prev_position =
      math::Vec3f(update_transform.transform().prev_position().x(),
                  update_transform.transform().prev_position().y(),
                  update_transform.transform().prev_position().z());
  auto& components = GlobalGameState().components;
  auto* transform_component =
      components.Get<component::TransformComponent>(
          update_transform.entity_id());
  if (transform_component) *transform_component = transform;
}


}

}
