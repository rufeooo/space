#include <cassert>
#include <thread>
#include <gflags/gflags.h>

#include "asteroids.h"
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

bool IsSinglePlayer() { return FLAGS_hostname.empty(); }

bool Initialize() {
  // Clients entities start at max free entity and then decrement.
  // These are largely ephemeral and will be deleted when the server
  // replicates client created entities.
  asteroids::SetEntityStart(ecs::ENTITY_LAST_FREE);
  asteroids::SetEntityIncrement(-1);

  // Add a single receipient for the server. Otherwise messages
  // can not be added to the queue :(
  //connection_component->outgoing_message_queue.AddRecipient(0);
  if (!asteroids::Initialize()) {
    std::cout << "Failed to initialize asteroids." << std::endl;
    return false;
  }

  auto* create_player = game::CreateEvent<asteroids::CreatePlayer>(
      asteroids::Event::CREATE_PLAYER);
  create_player->mutate_entity_id(asteroids::GenerateFreeEntity());
 
  if (IsSinglePlayer()) {
    asteroids::GlobalGameState().components
        .Assign<asteroids::GameStateComponent>(
            asteroids::GenerateFreeEntity());
  }

  return true;
}

bool ProcessInput() {
  glfwPollEvents();
  auto& opengl = asteroids::GlobalOpenGL();
  auto& components = asteroids::GlobalGameState().components;
  components.Enumerate<component::InputComponent>(
      [&opengl](ecs::Entity ent, component::InputComponent& input) {
    input.previous_input_mask = input.input_mask;
    input.input_mask = 0;
    if (glfwGetKey(opengl.glfw_window, GLFW_KEY_W)) {
      component::SetKeyDown(input.input_mask, component::KEYBOARD_W); 
    }
    if (glfwGetKey(opengl.glfw_window, GLFW_KEY_A)) {
      component::SetKeyDown(input.input_mask, component::KEYBOARD_A); 
    }
    if (glfwGetKey(opengl.glfw_window, GLFW_KEY_S)) {
      component::SetKeyDown(input.input_mask, component::KEYBOARD_S); 
    }
    if (glfwGetKey(opengl.glfw_window, GLFW_KEY_D)) {
      component::SetKeyDown(input.input_mask, component::KEYBOARD_D); 
    }
    if (glfwGetKey(opengl.glfw_window, GLFW_KEY_SPACE)) {
      component::SetKeyDown(input.input_mask,
                            component::KEYBOARD_SPACE); 
    }
  });
  return true;
}

void OnEnd() {
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
