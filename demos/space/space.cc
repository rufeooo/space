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
  // Make a square. This thing moves around when clicking.
  auto* transform = kECS.Assign<TransformComponent>(0);
  transform->position = math::Vec3f(400.f, 400.f, 0.f);
  kECS.Assign<RectangleComponent>(0);

  // Make a triangle. This doesn't really do anything.
  transform = kECS.Assign<TransformComponent>(1);
  transform->position = math::Vec3f(200.f, 200.f, 0.f);
  kECS.Assign<TriangleComponent>(1);

  return true;
}

bool ProcessInput() {
  gfx::PollEvents();

  if (gfx::LeftMouseClicked()) {
    math::Vec2f cursor_pos = gfx::GetCursorPositionInScreenSpace();
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
  // Rotate the triangle.
  auto* transform = kECS.Get<TransformComponent>(1);
  transform->orientation.Rotate(1.f);

  // Move the square towards the click.
  kECS.Enumerate<TransformComponent, DestinationComponent>(
      [](ecs::Entity ent, TransformComponent& transform,
         DestinationComponent& destination) {
    auto dir = math::Normalize(destination.position - transform.position.xy());
    // Box will jitter after reaching destination. DestinationComponent will
    // need to be removed when it "arrives" to fix that.
    transform.position += dir * 5.f;
  });

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
