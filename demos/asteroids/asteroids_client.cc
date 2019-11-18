#include <thread>
#include <gflags/gflags.h>

#include "asteroids.h"
#include "asteroids_commands.h"
#include "asteroids_state.h"
#include "components/common/transform_component.h"
#include "components/common/input_component.h"
#include "network/client.h"
#include "network/message_queue.h"
#include "protocol/asteroids_commands_generated.h"
#include "game/game.h"

#include "ecs/internal.h"

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

  asteroids::CreatePlayer create_player(
      asteroids::GenerateFreeEntity(),
      asteroids::Vec3(0.f, 0.f, 0.f));
  asteroids::commands::Execute(create_player);
 
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
  glfwPollEvents();
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

bool Update() {
  asteroids::UpdateGame();
  return true;
}

bool Render() {
  return asteroids::RenderGame();
}

void OnEnd() {
  //integration::entity_replication::Stop();
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  game::Setup(&Initialize, &ProcessInput, &Update, &Render, &OnEnd);


      
  if (!game::Run()) {
    std::cerr << "Encountered error running game..." << std::endl;
  }

  return 0;
}
