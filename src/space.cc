#include <cassert>

#include "camera.cc"
#include "command.cc"
#include "ecs.h"
#include "game/event_buffer.cc"
#include "gfx.cc"
#include "math/math.cc"

struct State {
  // Game and render updates per second
  uint64_t framerate = 60;
  // Calculated available microseconds per game_update
  uint64_t frame_target_usec;
  // Game Halt variable
  bool end = false;
  // Allow yielding idle cycles to kernel
  bool sleep_on_loop = true;
  // Number of times the game has been updated.
  uint64_t game_updates = 0;
  // Number of times the game frame was exceptionally delayed
  uint64_t game_jerk = 0;
  // ...
  game::EventBuffer event_buffer;
};

static State kGameState;

bool
Initialize()
{
  if (!gfx::Initialize()) return false;
  // Make a square. This thing moves around when clicking.
  auto* transform = kECS.Assign<TransformComponent>(0);
  transform->position = math::Vec3f(400.f, 400.f, 0.f);
  auto rect = kECS.Assign<RectangleComponent>(0);
  rect->color = math::Vec4f(1.f, 1.f, 1.f, 1.f);

  // Make a triangle. This doesn't really do anything.
  transform = kECS.Assign<TransformComponent>(1);
  transform->position = math::Vec3f(200.f, 200.f, 0.f);
  // transform->orientation.Set(90.f, math::Vec3f(0.f, 0.f, -1.f));
  auto* tri = kECS.Assign<TriangleComponent>(1);
  tri->color = math::Vec4f(1.f, 1.f, 1.f, 1.f);

  auto* grid = kECS.Assign<GridComponent>(2);
  grid->width = 50.f;
  grid->height = 50.f;
  grid->color = math::Vec4f(0.207f, 0.317f, 0.360f, 0.60f);

  grid = kECS.Assign<GridComponent>(3);
  grid->width = 25.f;
  grid->height = 25.f;
  grid->color = math::Vec4f(0.050f, 0.215f, 0.050f, 0.45f);

  return true;
}

bool
ProcessInput()
{
  PlatformEvent event;
  static math::Vec2f camera_translate(0.f, 0.f);
  while (window::PollEvent(&event)) {
    switch (event.type) {
      case MOUSE_DOWN: {
        if (event.button == BUTTON_LEFT) {
          command::Move* move =
              game::EnqueueEvent<command::Move>(command::MOVE);
          move->entity_id = 0;
          // A bit of an optimization. Assume no zoom when converting to world
          // space.
          move->position = event.position + camera::position().xy();
        }
      } break;
      case KEY_DOWN: {
        switch (event.key) {
          case 'w': {
            camera_translate.y = 1.f;
          } break;
          case 'a': {
            camera_translate.x = -1.f;
          } break;
          case 's': {
            camera_translate.y = -1.f;
          } break;
          case 'd': {
            camera_translate.x = 1.f;
          } break;
          default:
            break;
        }
      } break;
      case KEY_UP: {
        switch (event.key) {
          case 'w': {
            camera_translate.y = 0.f;
          } break;
          case 'a': {
            camera_translate.x = 0.f;
          } break;
          case 's': {
            camera_translate.y = 0.f;
          } break;
          case 'd': {
            camera_translate.x = 0.f;
          } break;
          default:
            break;
        }
      } break;
      default:
        break;
    }
  }
  camera::Translate(camera_translate);

  return true;
}

void
HandleEvent(game::Event event)
{
  switch ((command::Event)event.metadata) {
    case command::MOVE:
      command::Execute(*((command::Move*)event.data));
      break;
    case command::INVALID:
    default:
      assert("Invalid command.");
  }
}

bool
UpdateGame()
{
  TransformComponent* transform = kECS.Get<TransformComponent>(0);
  DestinationComponent* destination = kECS.Get<DestinationComponent>(0);
  if (destination) {
    auto dir =
        math::Normalize(destination->position - transform->position.xy());
    transform->position += dir * 1.f;
    float length_squared =
        math::LengthSquared(transform->position.xy() - destination->position);
    // Remove DestinationComponent so entity stops.
    if (length_squared < 15.0f) {
      kECS.Remove<DestinationComponent>(0);
    }
  }

  return true;
}

void
OnEnd()
{
}

constexpr int kEventBufferSize = 20 * 1024;

int
main(int argc, char** argv)
{
  uint64_t loop_count = 0;

  game::AllocateEventBuffer(kEventBufferSize);
  if (!Initialize()) {
    OnEnd();
    return 1;
  }

  kGameState.game_updates = 0;
  kGameState.game_jerk = 0;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;
  platform::clock_init();

  while (!window::ShouldClose() &&
         (loop_count == 0 || kGameState.game_updates < loop_count)) {
    if (kGameState.end) {
      OnEnd();
      return 1;
    }

    ProcessInput();

    {
      // Dequeue and handle all events in event queue.
      game::Event event;
      while (PollEvent(&event)) {
        HandleEvent(event);
      }

      // Clears all memory in event buffer since they should
      // have all been handled by now.
      game::ResetEventBuffer();

      // Give the user an update tick. The engine runs with
      // a fixed delta so no need to provide a delta time.
      UpdateGame();

      ++kGameState.game_updates;
    }

    if (!gfx::Render()) {
      OnEnd();
      return 1;
    }

    uint64_t sleep_usec = 0;
    while (!platform::elapse_usec(kGameState.frame_target_usec, &sleep_usec,
                                  &kGameState.game_jerk)) {
      if (kGameState.sleep_on_loop) platform::sleep_usec(sleep_usec);
    }
  }

  OnEnd();
  game::DeallocateEventBuffer();

  return 0;
}
