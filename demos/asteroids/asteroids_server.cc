#include <array>
#include <thread>
#include <gflags/gflags.h>
#include <cassert>

#include "asteroids.h"
#include "asteroids_state.h"
#include "network/server.h"
#include "network/message_queue.h"
#include "protocol/asteroids_commands_generated.h"
#include "game/game.h"

DEFINE_string(port, "9845", "Port for this application.");
// TODO: Reimplement.
DEFINE_bool(headless, true, "Set to false to render game. Should be "
                            "used for debug only");

class AsteroidsServer : public game::Game {
 public:
  AsteroidsServer() : game::Game() {};
  bool Initialize() override {
    assert(!FLAGS_port.empty());
    network_thread_ = network::server::Create(
        FLAGS_port.c_str(), &outgoing_message_queue_,
        &incoming_message_queue_);
    if (!asteroids::Initialize()) {
      std::cout << "Failed to initialize asteroids." << std::endl;
      return false;
    }
    asteroids::GlobalGameState().components
        .Assign<asteroids::GameStateComponent>(
            asteroids::GlobalFreeEntity()++);
    return true;
  }

  bool ProcessInput() override {
    glfwPollEvents();
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
      incoming_message_queue_.Stop();
      network_thread_.join();
    }
  }

 private:
  // Network related.
  network::OutgoingMessageQueue outgoing_message_queue_;
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
