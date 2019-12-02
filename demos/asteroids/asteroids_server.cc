#include <array>
#include <thread>
#include <gflags/gflags.h>
#include <cassert>

#include "asteroids.h"
#include "asteroids_commands.h"
#include "asteroids_state.h"
#include "network/server.h"
#include "protocol/asteroids_commands_generated.h"
#include "game/game.h"

DEFINE_string(port, "9845", "Port for this application.");

bool Initialize() {
  assert(!FLAGS_port.empty());

  if (!network::server::Start(FLAGS_port.c_str())) {
    std::cout << "Unable to start server." << std::endl;
    return 0;
  }

  if (!asteroids::Initialize()) {
    std::cout << "Failed to initialize asteroids." << std::endl;
    return false;
  }

  asteroids::GlobalGameState().components
      .Assign<asteroids::GameStateComponent>(
          asteroids::GenerateFreeEntity());
  return true;
}

bool ProcessInput() {
  glfwPollEvents();
  return true;
}

bool Update() {
  asteroids::UpdateGame();
  return true;
}

bool Render() {
  return asteroids::RenderGame();
}

void OnEnd() {
  network::server::Stop();
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  game::Setup(&Initialize,
              &ProcessInput,
              &asteroids::HandleEvent,
              &Update,
              &Render,
              &OnEnd);
  if (!game::Run()) {
    std::cerr << "Encountered error running game..." << std::endl;
  }
  return 0;
}
