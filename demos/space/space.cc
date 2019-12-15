#include <iostream>

#include "game/game.h"
#include "ecs.h"
#include "gfx.h"

namespace {

bool Initialize() {
  if (!gfx::Initialize()) return false;
  // Just make a player... It's a triangle.
  kECS.Assign<TransformComponent>(0);
  kECS.Assign<TriangleComponent>(0);
  return true;
}

bool ProcessInput() {
  gfx::PollEvents();
  return true;
}

void HandleEvent(game::Event event) {
}

bool UpdateGame() {
  return true;
}

void OnEnd() {
}

}

int main(int argc, char** argv) {
  game::Setup(&Initialize,
              &ProcessInput,
              &HandleEvent,
              &UpdateGame,
              &gfx::Render,
              &OnEnd);

  if (!game::Run()) {
    std::cerr << "Encountered error running spacey game..." << std::endl;
  }

  return 0;

}
