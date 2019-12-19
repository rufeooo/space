#include <iostream>
#include <cassert>

#include "game/game.h"
#include "game/event_buffer.h"
#include "command.h"
#include "ecs.h"
#include "gfx.h"
#include "math/vec.h"

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
  transform->orientation.Set(0.f, math::Vec3f(0.f, 0.f, 1.f));
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
  //auto* transform = kECS.Get<TransformComponent>(1);
  //transform->orientation.Rotate(1.f);

  TransformComponent* transform = kECS.Get<TransformComponent>(0);
  DestinationComponent* destination = kECS.Get<DestinationComponent>(0);
  if (destination) {
    auto dir =
        math::Normalize(destination->position - transform->position.xy());
    // Box will jitter after reaching destination. DestinationComponent will
    // need to be removed when it "arrives" to fix that.
    transform->position += dir * 1.f;
    float length_squared =
        math::LengthSquared(transform->position.xy() - destination->position);
    if (length_squared < 15.0f) {
      kECS.Remove<DestinationComponent>(0);
    }
  }

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
