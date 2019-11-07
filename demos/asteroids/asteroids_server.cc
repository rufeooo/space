#include <array>
#include <thread>
#include <gflags/gflags.h>
#include <cassert>

#include "asteroids.h"
#include "network/server.h"
#include "network/message_queue.h"
#include "protocol/asteroids_packet_generated.h"
#include "game/game.h"

DEFINE_string(port, "9843", "Port for this application.");
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
    return true;
  }

  bool Update() override {
    asteroids::UpdateGame(game_options_);
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
  // Game related.
  asteroids::Options game_options_;

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
