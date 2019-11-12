#include <array>
#include <thread>
#include <gflags/gflags.h>
#include <cassert>

#include "asteroids.h"
#include "asteroids_commands.h"
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
    auto* connection_component =
        asteroids::GlobalGameState().singleton_components
            .Get<asteroids::ConnectionComponent>();
    connection_component->network_thread = network::server::Create(
        FLAGS_port.c_str(),
        &connection_component->outgoing_message_queue,
        &connection_component->incoming_message_queue);
    connection_component->is_server = true;
    if (!asteroids::Initialize()) {
      std::cout << "Failed to initialize asteroids." << std::endl;
      return false;
    }
    asteroids::GlobalGameState().components
        .Assign<asteroids::GameStateComponent>(
            asteroids::GenerateFreeEntity());
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
    auto* connection_component =
        asteroids::GlobalGameState().singleton_components
            .Get<asteroids::ConnectionComponent>();
    if (connection_component->network_thread.joinable()) {
      connection_component->incoming_message_queue.Stop();
      connection_component->network_thread.join();
    }
  }
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  AsteroidsServer asteroids_server;
  asteroids_server.Run();
  return 0;
}
