#include <array>
#include <thread>
#include <gflags/gflags.h>
#include <cassert>

#include "network/server.h"
#include "network/message_queue.h"
#include "game/game.h"

DEFINE_string(port, "1843", "Port for this application.");

class AsteroidsClient : public game::Game {
 public:
  AsteroidsClient() : game::Game() {};
  bool Initialize() override {
    assert(!FLAGS_port.empty());
    network_thread_ = network::server::Create(
        FLAGS_port.c_str(), &outgoing_message_queue_,
        &incoming_message_queue_);
    return true;
  }

  bool ProcessInput() override {
    return true;
  }

  bool Update() override {
    return true;
  }

  bool Render() override {
    return true;
  }

 private:
  // Network related.
  std::array<network::OutgoingMessageQueue, network::kMaxClients>
    outgoing_message_queue_;
  network::IncomingMessageQueue incoming_message_queue_;
  std::thread network_thread_;
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  AsteroidsServer asteroids_server;
  asteroids_server.Run();
  return 0;
}
