#include <thread>
#include <gflags/gflags.h>

#include "asteroids.h"
#include "components/common/transform_component.h"
#include "network/client.h"
#include "network/message_queue.h"
#include "protocol/asteroids_packet_generated.h"
#include "game/game.h"

#include "ecs/internal.h"

DEFINE_string(hostname, "",
              "If provided will connect to a game server. Will play "
              "the game singleplayer otherwise.");
DEFINE_string(port, "1843", "Port for this application.");

bool IsSinglePlayer() { return FLAGS_hostname.empty(); }

class AsteroidsClient : public game::Game {
 public:
  AsteroidsClient() : game::Game() {}

  bool Initialize() override {
    if (!FLAGS_hostname.empty()) {
      network_thread_ = network::client::Create(
          FLAGS_hostname.c_str(), FLAGS_port.c_str(),
          &outgoing_message_queue_, &incoming_message_queue_);
    }
    game_options_.opengl = asteroids::OpenGL();
    if (!asteroids::Initialize(game_options_)) {
      std::cout << "Failed to initialize asteroids." << std::endl;
      return false;
    }
    player_ = asteroids::SpawnPlayer(game_options_,
                                     math::Vec3f(0.f, 0.f, 0.f));
    game_options_.game_state.components
        .Assign<asteroids::InputComponent>(player_);
    if (IsSinglePlayer()) {
      game_options_.game_state.components
        .Assign<asteroids::GameStateComponent>(
            game_options_.free_entity++);
    }
    return true;
  }

  bool ProcessInput() override {
    asteroids::ProcessClientInput(game_options_);
    SendPlayerState();
    ReceiveServerState();
    return true;
  }

  bool Update() override {
    asteroids::UpdateGame(game_options_);
    return true;
  }

  bool Render() override {
    return asteroids::RenderGame(game_options_);
  }

  void OnGameEnd() override {
    if (network_thread_.joinable()) {
      outgoing_message_queue_.Stop();
      network_thread_.join();
    }
  }

 private:
  void SendPlayerState() {
    // Singleplayer games don't send data to the server.
    if (IsSinglePlayer()) return;
    auto& transform =
        *game_options_.game_state.components
            .Get<component::TransformComponent>(player_);
    flatbuffers::FlatBufferBuilder fbb;
    auto position = asteroids::Vec3(
        transform.position.x(), transform.position.y(),
        transform.position.z());
    auto orientation = asteroids::Vec4(
        transform.orientation.x(), transform.orientation.y(),
        transform.orientation.z(), transform.orientation.w());
    auto player_transform = asteroids::Transform(
        position, orientation);
    auto& physics = *game_options_.game_state.components
      .Get<asteroids::PhysicsComponent>(player_);
    auto fb_acceleration = asteroids::Vec3(
        physics.acceleration.x(), physics.acceleration.y(),
        physics.acceleration.z());
    auto fb_velocity = asteroids::Vec3(
        physics.velocity.x(), physics.velocity.y(),
        physics.velocity.z());
    auto player_physics = asteroids::Physics(
        fb_acceleration, fb_velocity); 
    auto player_state = asteroids::PlayerState(
        player_, player_transform, player_physics);
    auto packet = asteroids::CreatePacket(fbb, &player_state, nullptr);
    fbb.Finish(packet);
    outgoing_message_queue_.Enqueue(fbb.Release());
  }

  void HandlePlayerState(const asteroids::PlayerState* player_state) {
    if (!player_state) return;
    uint64_t server_entity = player_state->entity_id();
    auto client_entity_itr =
      server_entity_mappings_.find(server_entity);
  }

  void HandleAsteroidState(
      const asteroids::AsteroidState* asteroid_state) {
    if (!asteroid_state) return;
    auto& components = game_options_.game_state.components;
    uint64_t server_entity = asteroid_state->entity_id();
    auto client_entity_itr =
      server_entity_mappings_.find(server_entity);
    ecs::Entity client_entity;
    // Check if the server knows about this client entity.
    if (client_entity_itr == server_entity_mappings_.end()) {
      ecs::Entity asteroid_entity = SpawnAsteroid(
          game_options_, math::Vec3f(0.f, 0.f, 0.f),
          math::Vec3f(0.f, 0.f, 0.f), 0.f,
          asteroid_state->random_number());
      // The server doesn't know  about this entity so make it.
      client_entity = asteroid_entity;
      server_entity_mappings_[server_entity] = client_entity;
    } else { client_entity = client_entity_itr->second; }
    auto fb_physics = asteroid_state->physics();
    asteroids::PhysicsComponent* asteroid_physics =
        components.Get<asteroids::PhysicsComponent>(client_entity);
    if (!asteroid_physics) return;
    asteroid_physics->acceleration = math::Vec3f(
        fb_physics.acceleration().x(),
        fb_physics.acceleration().y(),
        fb_physics.acceleration().z());
    asteroid_physics->velocity = math::Vec3f(
        fb_physics.velocity().x(),
        fb_physics.velocity().y(),
        fb_physics.velocity().z());
    auto fb_transform = asteroid_state->transform();
    component::TransformComponent* asteroid_transform =
        components.Get<component::TransformComponent>(client_entity);
    if (!asteroid_transform) return;
    asteroid_transform->position = math::Vec3f(
        fb_transform.position().x(),
        fb_transform.position().y(),
        fb_transform.position().z());
    //std::cout << "Set: " << server_entity << " position "
    //          << player_transform->position.String() << std::endl;
    asteroid_transform->orientation = math::Quatf(
        fb_transform.orientation().x(),
        fb_transform.orientation().y(),
        fb_transform.orientation().z(),
        fb_transform.orientation().w());
  }

  void ReceiveServerState() {
    auto& components = game_options_.game_state.components;
    network::Message msg = incoming_message_queue_.Dequeue();
    while (msg.size != 0) {
      // Deserialize the message into a flatbuffer.
      const asteroids::Packet* asteroids_packet = asteroids::GetPacket(
          (const void*)msg.data);
      HandlePlayerState(asteroids_packet->player_state());
      HandleAsteroidState(asteroids_packet->asteroid_state());
      free(msg.data);  // Probably not ideal to do this here.
      msg = incoming_message_queue_.Dequeue();
    }
  }

  // Game related.
  asteroids::Options game_options_;
  ecs::Entity player_;

  // Network related.
  network::OutgoingMessageQueue outgoing_message_queue_;
  network::IncomingMessageQueue incoming_message_queue_;
  std::thread network_thread_;

  // Server Entity Mappings.
  std::unordered_map<ecs::Entity, ecs::Entity> server_entity_mappings_;
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  AsteroidsClient asteroids_client;
  asteroids_client.Run();
  return 0;
}
