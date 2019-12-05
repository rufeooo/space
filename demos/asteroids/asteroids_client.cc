#include <cassert>
#include <thread>
#include <gflags/gflags.h>

#include "asteroids.h"
#include "asteroids_commands.h"
#include "asteroids_state.h"
#include "components/common/transform_component.h"
#include "components/common/input_component.h"
#include "ecs/internal.h"
#include "game/event_buffer.h"
#include "game/game.h"

DEFINE_string(hostname, "",
              "If provided will connect to a game server. Will play "
              "the game singleplayer otherwise.");
DEFINE_string(port, "9845", "Port for this application.");
DEFINE_string(replay_file, "", "Run game from replay file.");

void OnServerMsgReceived(uint8_t* msg, int size) {
}

bool IsSinglePlayer() { return FLAGS_hostname.empty(); }

bool Initialize() {
  if (!FLAGS_hostname.empty()) {
    network::client::Setup(&OnServerMsgReceived);
    if (!network::client::Start(
        FLAGS_hostname.c_str(), FLAGS_port.c_str())) {
      std::cout << "Unable to start client." << std::endl;
      return false;
    }
  }
  // Clients entities start at max free entity and then decrement.
  // These are largely ephemeral and will be deleted when the server
  // replicates client created entities.
  asteroids::SetEntityStart(ecs::ENTITY_LAST_FREE);
  asteroids::SetEntityIncrement(-1);

  if (!asteroids::Initialize()) {
    std::cout << "Failed to initialize asteroids." << std::endl;
    return false;
  }

  game::SaveEventsToFile();

  auto* create_player = game::CreateEvent<asteroids::CreatePlayer>(
      asteroids::Event::CREATE_PLAYER);
  create_player->entity_id = asteroids::GenerateFreeEntity();

  // Inform the server of this player joining.
  network::client::Send(
      ((uint8_t*)create_player - game::kEventHeaderSize),
      sizeof(asteroids::CreatePlayer) + game::kEventHeaderSize);
 
  if (IsSinglePlayer()) {
    asteroids::GlobalGameState().components
        .Assign<asteroids::GameStateComponent>(
            asteroids::GenerateFreeEntity());
  }

  if (!FLAGS_replay_file.empty()) {
    game::LoadEventsFromFile(FLAGS_replay_file.c_str());
  }

  return true;
}

bool ProcessInput() {
  glfwPollEvents();
  static asteroids::Input previous_player_input;
  static asteroids::Input player_input;
  previous_player_input = player_input;
  auto& opengl = asteroids::GlobalOpenGL();
  player_input.previous_input_mask = player_input.input_mask;
  player_input.input_mask = 0;
  if (glfwGetKey(opengl.glfw_window, GLFW_KEY_W)) {
    player_input.input_mask =
        player_input.input_mask | (uint8_t)asteroids::InputKey::W;
  }
  if (glfwGetKey(opengl.glfw_window, GLFW_KEY_A)) {
    player_input.input_mask =
        player_input.input_mask | (uint8_t)asteroids::InputKey::A;
  }
  if (glfwGetKey(opengl.glfw_window, GLFW_KEY_S)) {
    player_input.input_mask =
        player_input.input_mask | (uint8_t)asteroids::InputKey::S;
  }
  if (glfwGetKey(opengl.glfw_window, GLFW_KEY_D)) {
    player_input.input_mask =
        player_input.input_mask | (uint8_t)asteroids::InputKey::D;
  }
  if (glfwGetKey(opengl.glfw_window, GLFW_KEY_SPACE)) {
    player_input.input_mask =
        player_input.input_mask | (uint8_t)asteroids::InputKey::SPACE;
  }
  if (player_input.input_mask !=
          previous_player_input.input_mask ||
      player_input.previous_input_mask !=
          previous_player_input.previous_input_mask) {
    *game::CreateEvent<asteroids::Input>(
        asteroids::Event::PLAYER_INPUT) = player_input;
  }
  return true;
}

void OnEnd() {
  network::client::Stop();
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  game::Setup(&Initialize, &ProcessInput,
              &asteroids::HandleEvent,
              &asteroids::UpdateGame,
              &asteroids::RenderGame,
              &OnEnd);
  if (!game::Run()) {
    std::cerr << "Encountered error running game..." << std::endl;
  }

  return 0;
}
