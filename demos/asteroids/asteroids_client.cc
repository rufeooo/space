#include <thread>
#include <gflags/gflags.h>

#include "asteroids.h"
#include "asteroids_commands.h"
#include "asteroids_state.h"
#include "components/common/transform_component.h"
#include "network/client.h"
#include "network/message_queue.h"
#include "protocol/asteroids_commands_generated.h"
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
    if (!asteroids::Initialize()) {
      std::cout << "Failed to initialize asteroids." << std::endl;
      return false;
    }
    asteroids::CreatePlayer create_player(
        asteroids::GlobalFreeEntity()++,
        asteroids::Vec3(0.f, 0.f, 0.f));
    asteroids::commands::Execute(create_player);
    if (IsSinglePlayer()) {
      asteroids::GlobalGameState().components
          .Assign<asteroids::GameStateComponent>(
              asteroids::GlobalFreeEntity()++);
    }
    return true;
  }

  bool ProcessInput() override {
    asteroids::ProcessClientInput();
    return true;
  }

  bool Update() override {
    asteroids::UpdateGame();
    return true;
  }

  bool Render() override {
    return asteroids::RenderGame();
  }

  void OnGameEnd() override {
    if (network_thread_.joinable()) {
      outgoing_message_queue_.Stop();
      network_thread_.join();
    }
  }

 private:
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
