#include <array>
#include <thread>
#include <gflags/gflags.h>
#include <cassert>

#include "asteroids.h"
#include "asteroids_commands.h"
#include "asteroids_state.h"
#include "game/event_buffer.h"
#include "game/game.h"
#include "network/server.h"

namespace {

DEFINE_string(port, "9845", "Port for this application.");

static uint64_t kClientPlayers[network::server::kMaxClients];

void OnClientConnected(int client_id) {
  std::cout << "Client: " << client_id << " connected." << std::endl;
}

void OnClientMsgReceived(int client_id, uint8_t* msg, int size) {
  game::Event e = game::Decode(msg);
  switch ((asteroids::commands::Event)e.metadata) {
    case asteroids::commands::CREATE_PLAYER: {
      // Create the the player with a server ID and update that
      // on the client.
      auto* create_player =
          game::CreateEvent<asteroids::commands::CreatePlayer>(
              asteroids::commands::CREATE_PLAYER);
      create_player->entity_id = asteroids::GenerateFreeEntity();

      // Send the client a delete and create to synchronize entity id.
      constexpr int size = sizeof(asteroids::commands::DeleteEntity) +
                           sizeof(asteroids::commands::CreatePlayer) +
                           2 * game::kEventHeaderSize;
      constexpr int create_idx = sizeof(asteroids::commands::DeleteEntity)
                                 + game::kEventHeaderSize;
      uint8_t data[size];

      // Decode incoming message to create player.
      game::Event e = game::Decode(msg);
      asteroids::commands::CreatePlayer* c =
          (asteroids::commands::CreatePlayer*)(e.data);

      // Setup responding message to delete that entity.
      asteroids::commands::DeleteEntity delete_entity;
      delete_entity.entity_id = c->entity_id;
      game::Encode(sizeof(asteroids::commands::DeleteEntity),
                   asteroids::commands::DELETE_ENTITY,
                   (uint8_t*)(&delete_entity),
                   (uint8_t*)(&data[0]));
      game::Encode(sizeof(asteroids::commands::CreatePlayer),
                   asteroids::commands::CREATE_PLAYER,
                   (uint8_t*)(&create_player[0]),
                   (uint8_t*)(&data[create_idx]));

      // Send message back to client.
      network::server::Send(client_id, (uint8_t*)&data[0], size);

      // Save off client_id -> ship entity_id mapping.
      kClientPlayers[client_id] = create_player->entity_id;
      break;
    }
    case asteroids::commands::PLAYER_INPUT: {
      // TODO: Do not directly enqueue client inputs.
      game::EnqueueEvent(msg, size);
    }
    default:
      std::cout << "Unhandled client message: " << e.metadata << std::endl;
  }
}

bool Initialize() {
  assert(!FLAGS_port.empty());

  network::server::Setup(&OnClientConnected, &OnClientMsgReceived);

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
