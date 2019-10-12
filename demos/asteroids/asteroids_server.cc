#include <array>
#include <thread>
#include <gflags/gflags.h>
#include <cassert>

#include "asteroids.h"
#include "network/server.h"
#include "network/message_queue.h"
#include "protocol/asteroids_packet_generated.h"
#include "game/game.h"

DEFINE_string(port, "1843", "Port for this application.");
DEFINE_bool(headless, true, "Set to false to render game. Should be "
                            "used for debug only");

bool IsHeadless() {
  return FLAGS_headless;
}

class AsteroidsServer : public game::Game {
 public:
  AsteroidsServer() : game::Game() {};
  bool Initialize() override {
    assert(!FLAGS_port.empty());
    network_thread_ = network::server::Create(
        FLAGS_port.c_str(), &outgoing_message_queue_,
        &incoming_message_queue_);
    if (!IsHeadless()) {
      game_options_.opengl = asteroids::OpenGL();
    }
    if (!asteroids::Initialize(game_options_)) {
      std::cout << "Failed to initialize asteroids." << std::endl;
      return false;
    }

    game_options_.game_state.components
        .Assign<asteroids::GameStateComponent>(
            game_options_.free_entity++);
    return true;
  }

  bool ProcessInput() override {
    if (!IsHeadless()) glfwPollEvents();
    ReceivePlayerState();
    return true;
  }

  bool Update() override {
    asteroids::UpdateGame(game_options_);
    SendServerState();
    return true;
  }

  bool Render() override {
    if (!IsHeadless()) return asteroids::RenderGame(game_options_);
    return true;
  }

  void OnGameEnd() override {
    if (network_thread_.joinable()) {
      incoming_message_queue_.Stop();
      network_thread_.join();
    }
  }

 private:
  void ReceivePlayerState() {
    auto& components = game_options_.game_state.components;
    network::Message msg = incoming_message_queue_.Dequeue();
    while (msg.size != 0) {
      // Deserialize the message into a flatbuffer.
      auto* asteroids_packet = asteroids::GetPacket(
          (const void*)msg.data);
      // Get the entity mapping.
      auto& entity_mapping = client_entity_mappings_[msg.client_id];
      //std::cout << msg.client_id << std::endl;
      auto* player_state = asteroids_packet->player_state();
      uint64_t client_entity = player_state->entity_id();
      auto server_entity_itr = entity_mapping.find(client_entity);
      ecs::Entity server_entity;
      // Check if the server knows about this client entity.
      if (server_entity_itr == entity_mapping.end()) {
        // The server doesn't know  about this entity so make it.
        server_entity = game_options_.free_entity;
        entity_mapping[client_entity] = server_entity;
        SpawnPlayer(game_options_, math::Vec3f());
        std::cout << "Server made player entity: "
                  << server_entity << std::endl;
      } else { server_entity = server_entity_itr->second; }
      // Deserialize data into local entity.
      auto fb_physics = player_state->physics();
      asteroids::PhysicsComponent* player_physics =
          components.Get<asteroids::PhysicsComponent>(server_entity);
      player_physics->acceleration = math::Vec3f(
          fb_physics.acceleration().x(),
          fb_physics.acceleration().y(),
          fb_physics.acceleration().z());
      player_physics->velocity = math::Vec3f(
          fb_physics.velocity().x(),
          fb_physics.velocity().y(),
          fb_physics.velocity().z());
      auto fb_transform = player_state->transform();
      component::TransformComponent* player_transform =
          components.Get<component::TransformComponent>(server_entity);
      player_transform->position = math::Vec3f(
          fb_transform.position().x(),
          fb_transform.position().y(),
          fb_transform.position().z());
      //std::cout << "Set: " << server_entity << " position "
      //          << player_transform->position.String() << std::endl;
      player_transform->orientation = math::Quatf(
          fb_transform.orientation().x(),
          fb_transform.orientation().y(),
          fb_transform.orientation().z(),
          fb_transform.orientation().w());
      free(msg.data);  // Probably not ideal to do this here.
      msg = incoming_message_queue_.Dequeue();
    }
  }

  void SendServerState() {
    for (const auto& asteroid_entity :
        game_options_.game_state.asteroid_entities) {
      auto& transform =
          *game_options_.game_state.components
              .Get<component::TransformComponent>(asteroid_entity);
      flatbuffers::FlatBufferBuilder fbb;
      auto position = asteroids::Vec3(
          transform.position.x(), transform.position.y(),
          transform.position.z());
      auto orientation = asteroids::Vec4(
          transform.orientation.x(), transform.orientation.y(),
          transform.orientation.z(), transform.orientation.w());
      auto asteroid_transform = asteroids::Transform(
          position, orientation);
      auto& physics = *game_options_.game_state.components
          .Get<asteroids::PhysicsComponent>(asteroid_entity);
      auto fb_acceleration = asteroids::Vec3(
          physics.acceleration.x(), physics.acceleration.y(),
          physics.acceleration.z());
      auto fb_velocity = asteroids::Vec3(
          physics.velocity.x(), physics.velocity.y(),
          physics.velocity.z());
      auto asteroid_physics = asteroids::Physics(
          fb_acceleration, fb_velocity); 
      auto& random_number = *game_options_.game_state.components
          .Get<asteroids::RandomNumberIntChoiceComponent>(
              asteroid_entity);
      auto asteroid_state = asteroids::AsteroidState(
          asteroid_entity, asteroid_transform, asteroid_physics,
          random_number.random_number);
      auto packet = asteroids::CreatePacket(
          fbb, nullptr, &asteroid_state);
      fbb.Finish(packet);
      for (const auto& client : client_entity_mappings_) {
        outgoing_message_queue_[client.first].Enqueue(fbb.Release());
      }
    }
  }

  // Game related.
  asteroids::Options game_options_;

  // Network related.
  std::array<network::OutgoingMessageQueue,
             network::server::kMaxClients> outgoing_message_queue_;
  network::IncomingMessageQueue incoming_message_queue_;
  std::thread network_thread_;

  // Client Entity Mappings.
  std::unordered_map<
      int, std::unordered_map<ecs::Entity, ecs::Entity>>
          client_entity_mappings_;
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  AsteroidsServer asteroids_server;
  asteroids_server.Run();
  return 0;
}
