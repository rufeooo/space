#include <iostream>
#include <cassert>

#include "game/game.h"
#include "game/event_buffer.h"
#include "command.h"
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

  if (gfx::LeftMouseClicked()) {
    math::Vec2f cursor_pos = gfx::GetCursorPositionInGLSpace();
    command::Move* move = game::CreateEvent<command::Move>(command::MOVE);
    move->entity_id = 0;
    move->position = cursor_pos;
  }

  return true;
}

void HandleEvent(game::Event event) {
  switch ((command::Event)event.metadata) {
    case command::MOVE:
      command::Execute(*((command::Move*)event.data));
      break;
    case command::INVALID:
    default:
      assert("Invalid command.");
  }
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
