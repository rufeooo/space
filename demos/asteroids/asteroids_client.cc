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
DEFINE_string(port, "9843", "Port for this application.");

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
    // Add a single receipient for the server. Otherwise messages
    // can not be added to the queue :(
    outgoing_message_queue_.AddRecipient(0);
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
