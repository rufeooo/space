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
    assert(game_options_.game_state.components
        .Get<asteroids::PhysicsComponent>(player_) != nullptr);
    assert(game_options_.game_state.components
        .Get<asteroids::InputComponent>(player_) != nullptr);
    assert(game_options_.game_state.components
        .Get<component::TransformComponent>(player_) != nullptr);
    asteroids::ProcessClientInput(game_options_);
    SendPlayerState();
    return true;
  }

  bool Update() override {
    asteroids::UpdateGame(game_options_);
    return true;
  }

  bool Render() override {
    return asteroids::RenderGame(game_options_);
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
    auto packet = asteroids::CreatePacket(
        fbb, &player_state, game_updates());
    fbb.Finish(packet);
    outgoing_message_queue_.Enqueue(fbb.Release());
  }

  // Game related.
  asteroids::Options game_options_;
  ecs::Entity player_;

  // Network related.
  network::OutgoingMessageQueue outgoing_message_queue_;
  network::IncomingMessageQueue incoming_message_queue_;
  std::thread network_thread_;
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  AsteroidsClient asteroids_client;
  asteroids_client.Run();
  return 0;
}
