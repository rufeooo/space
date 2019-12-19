#include <gflags/gflags.h>
#include <cassert>
#include <thread>

#include "asteroids/asteroids.cc"
#include "asteroids/asteroids_commands.cc"
#include "asteroids/asteroids_state.cc"
#include "components/common/input_component.h"
#include "components/common/transform_component.h"
#include "components/network/server_authoritative_component.h"
#include "ecs/internal.h"
#include "game/game.cc"
#include "math/math.cc"
#include "network/client.cc"
#include "platform/platform.cc"
#include "renderer/renderer.cc"

DEFINE_string(hostname, "",
              "If provided will connect to a game server. Will play "
              "the game singleplayer otherwise.");
DEFINE_string(port, "9845", "Port for this application.");
DEFINE_string(replay_file, "", "Run game from replay file.");

void
OnServerMsgReceived(uint8_t* msg, int size)
{
  assert(size != 0);
  game::EnqueueEvent(msg, size);
}

void
OnServerAuthorityCreated(ecs::Entity entity)
{
  constexpr int size = sizeof(asteroids::commands::ClientCreateAuthoritative) +
                       game::kEventHeaderSize;
  static uint8_t data[size];
  asteroids::commands::ClientCreateAuthoritative create;
  create.entity_id = entity;
  game::Encode(sizeof(asteroids::commands::ClientCreateAuthoritative),
               asteroids::commands::CLIENT_CREATE_AUTHORITATIVE,
               (uint8_t*)(&create), &data[0]);
  network::client::Send(&data[0], size);
}

void
OnServerAuthorityRemoved(ecs::Entity entity)
{
  constexpr int size = sizeof(asteroids::commands::ClientDeleteAuthoritative) +
                       game::kEventHeaderSize;
  static uint8_t data[size];
  asteroids::commands::ClientDeleteAuthoritative create;
  create.entity_id = entity;
  game::Encode(sizeof(asteroids::commands::ClientDeleteAuthoritative),
               asteroids::commands::CLIENT_DELETE_AUTHORITATIVE,
               (uint8_t*)(&create), &data[0]);
  network::client::Send(&data[0], size);
}

bool
SetupClientConnection()
{
  if (FLAGS_hostname.empty()) return true;

  network::client::Setup(&OnServerMsgReceived);
  if (!network::client::Start(FLAGS_hostname.c_str(), FLAGS_port.c_str())) {
    std::cout << "Unable to start client." << std::endl;
    return false;
  }

  return true;
}

void
SetupClientConfiguration()
{
  // Clients entities start at max free entity and then decrement.
  // These are largely ephemeral and will be deleted when the server
  // replicates client created entities.
  asteroids::SetEntityStart(ecs::ENTITY_LAST_FREE);
  asteroids::SetEntityIncrement(-1);

  // Writes all events to file in _tmp/<timestamp>. That way a session
  // can be replayed using the --replay_file flag.
  game::SaveEventsToFile();

  if (!FLAGS_replay_file.empty()) {
    game::LoadEventsFromFile(FLAGS_replay_file.c_str());
  }

  auto& components = asteroids::GlobalGameState().components;

  if (!FLAGS_hostname.empty()) {
    components.AssignCallback<ServerAuthoritativeComponent>(
        &OnServerAuthorityCreated);

    components.RemoveCallback<ServerAuthoritativeComponent>(
        &OnServerAuthorityRemoved);

    return;
  }

  // Only the server has a game state component in a networked game.
  components.Assign<asteroids::GameStateComponent>(
      asteroids::GenerateFreeEntity());
}

void
SetupClientPlayer()
{
  // Create the player.
  auto* create_player = game::CreateEvent<asteroids::commands::CreatePlayer>(
      asteroids::commands::CREATE_PLAYER);
  create_player->entity_id = asteroids::GenerateFreeEntity();

  // Set this clients player id to the right player id.
  auto* change_id = game::CreateEvent<asteroids::commands::PlayerIdMutation>(
      asteroids::commands::PLAYER_ID_MUTATION);
  change_id->entity_id = create_player->entity_id;

  if (FLAGS_hostname.empty()) return;

  // Inform the server of this player joining.
  network::client::Send(
      ((uint8_t*)create_player - game::kEventHeaderSize),
      sizeof(asteroids::commands::CreatePlayer) + game::kEventHeaderSize);
}

bool
Initialize()
{
  if (!asteroids::Initialize()) {
    std::cout << "Failed to initialize asteroids." << std::endl;
    return false;
  }

  if (!SetupClientConnection()) {
    return false;
  }

  SetupClientConfiguration();

  SetupClientPlayer();

  return true;
}

bool
ProcessInput()
{
  glfwPollEvents();
  static asteroids::commands::Input previous_player_input;
  static asteroids::commands::Input player_input;
  previous_player_input = player_input;
  auto& opengl = asteroids::GlobalOpenGL();
  player_input.previous_input_mask = player_input.input_mask;
  player_input.input_mask = 0;
  if (glfwGetKey(opengl.glfw_window, GLFW_KEY_W)) {
    player_input.input_mask =
        player_input.input_mask | (uint8_t)asteroids::commands::InputKey::W;
  }
  if (glfwGetKey(opengl.glfw_window, GLFW_KEY_A)) {
    player_input.input_mask =
        player_input.input_mask | (uint8_t)asteroids::commands::InputKey::A;
  }
  if (glfwGetKey(opengl.glfw_window, GLFW_KEY_S)) {
    player_input.input_mask =
        player_input.input_mask | (uint8_t)asteroids::commands::InputKey::S;
  }
  if (glfwGetKey(opengl.glfw_window, GLFW_KEY_D)) {
    player_input.input_mask =
        player_input.input_mask | (uint8_t)asteroids::commands::InputKey::D;
  }
  if (glfwGetKey(opengl.glfw_window, GLFW_KEY_SPACE)) {
    player_input.input_mask =
        player_input.input_mask | (uint8_t)asteroids::commands::InputKey::SPACE;
  }
  if (player_input.input_mask != previous_player_input.input_mask ||
      player_input.previous_input_mask !=
          previous_player_input.previous_input_mask) {
    auto* input_event = game::CreateEvent<asteroids::commands::Input>(
        asteroids::commands::PLAYER_INPUT);
    *input_event = player_input;
    input_event->entity_id = asteroids::GlobalGameState().player_id;

    // Send player input to the server.
    network::client::Send(
        ((uint8_t*)input_event - game::kEventHeaderSize),
        sizeof(asteroids::commands::Input) + game::kEventHeaderSize);
  }
  return true;
}

void
OnEnd()
{
  network::client::Stop();
}

// namespace

int
main(int argc, char** argv)
{
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  game::Setup(&Initialize, &ProcessInput, &asteroids::HandleEvent,
              &asteroids::UpdateGame, &asteroids::RenderGame, &OnEnd);
  if (!game::Run()) {
    std::cerr << "Encountered error running game..." << std::endl;
  }

  return 0;
}
