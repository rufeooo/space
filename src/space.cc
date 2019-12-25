#include <cassert>

#include "math/math.cc"

#include "space/camera.cc"
#include "space/command.cc"
#include "space/ecs.cc"
#include "space/gfx.cc"

#define MAX_TICK_EVENTS 30
#define PAGE (4 * 1024)

struct State {
  // Game and render updates per second
  uint64_t framerate = 60;
  // Calculated available microseconds per game_update
  uint64_t frame_target_usec;
  // Allow yielding idle cycles to kernel
  bool sleep_on_loop = true;
  // Number of times the game has been updated.
  uint64_t game_updates = 0;
  // Number of times the game frame was exceptionally delayed
  uint64_t game_jerk = 0;
  // Events handled per input tick
  PlatformEvent ievent[MAX_TICK_EVENTS];
  uint64_t used_ievent = 0;
  // Network resources
  Udp4 socket;
  uint8_t netbuffer[PAGE];
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
Connect()
{
  if (!udp::Init()) return false;

  if (!udp::GetAddr4("localhost", "9845", &kGameState.socket)) return false;

  return true;
}

void
ProcessLocalInput()
{
  PlatformEvent event;
  for (kGameState.used_ievent = 0; kGameState.used_ievent < MAX_TICK_EVENTS;
       ++kGameState.used_ievent) {
    if (!window::PollEvent(&kGameState.ievent[kGameState.used_ievent])) break;
  }
}

void
NetworkEgress()
{
  // write frame
  *((int*)kGameState.netbuffer) = kGameState.game_updates;
  // write input
  memcpy(kGameState.netbuffer + sizeof(int), kGameState.ievent,
         sizeof(PlatformEvent) * kGameState.used_ievent);

  if (!udp::Send(
          kGameState.socket, kGameState.netbuffer,
          sizeof(int) + sizeof(PlatformEvent) * kGameState.used_ievent)) {
    exit(1);
  }
}

void
NetworkIngress()
{
  uint16_t bytes_received;
  if (!udp::ReceiveFrom(kGameState.socket, sizeof(kGameState.netbuffer),
                        kGameState.netbuffer, &bytes_received))
    return;

  if (bytes_received > sizeof(kGameState.ievent)) exit(1);

  // TODO: packet reordering

  memcpy(kGameState.ievent, kGameState.netbuffer + sizeof(int),
         bytes_received - 4);
  kGameState.used_ievent = (bytes_received - 4) / sizeof(PlatformEvent);
}

void
GameInput(PlatformEvent* event, math::Vec2f* camera)
{
  switch (event->type) {
    case MOUSE_DOWN: {
      if (event->button == BUTTON_LEFT) {
        command::Move move;
        move.entity_id = 0;
        // A bit of an optimization. Assume no zoom when converting to world
        // space.
        move.position = event->position + camera::position().xy();
        command::Execute(move);
      }
    } break;
    case KEY_DOWN: {
      switch (event->key) {
        case 'w': {
          camera->y = 1.f;
        } break;
        case 'a': {
          camera->x = -1.f;
        } break;
        case 's': {
          camera->y = -1.f;
        } break;
        case 'd': {
          camera->x = 1.f;
        } break;
        default:
          break;
      }
    } break;
    case KEY_UP: {
      switch (event->key) {
        case 'w': {
          camera->y = 0.f;
        } break;
        case 'a': {
          camera->x = 0.f;
        } break;
        case 's': {
          camera->y = 0.f;
        } break;
        case 'd': {
          camera->x = 0.f;
        } break;
        default:
          break;
      }
    } break;
    default:
      break;
  }
}

void
ProcessGameInput(uint64_t event_count, PlatformEvent* event)
{
  static math::Vec2f camera_translate(0.f, 0.f);
  for (int i = 0; i < event_count; ++i) {
    GameInput(&event[i], &camera_translate);
  }
  camera::Translate(camera_translate);
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

constexpr int kEventBufferSize = 20 * 1024;

int
main()
{
  uint64_t loop_count = 0;

  if (!Initialize()) {
    return 1;
  }

  if (!Connect()) {
    return 1;
  }

  kGameState.game_updates = 0;
  kGameState.game_jerk = 0;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;
  platform::clock_init();

  while (!window::ShouldClose() &&
         (loop_count == 0 || kGameState.game_updates < loop_count)) {
    ProcessLocalInput();
    NetworkEgress();
    NetworkIngress();
    ProcessGameInput(kGameState.used_ievent, kGameState.ievent);

    {
      // Give the user an update tick. The engine runs with
      // a fixed delta so no need to provide a delta time.
      UpdateGame();

      ++kGameState.game_updates;
    }

    if (!gfx::Render()) {
      return 1;
    }

    uint64_t sleep_usec = 0;
    while (!platform::elapse_usec(kGameState.frame_target_usec, &sleep_usec,
                                  &kGameState.game_jerk)) {
      if (kGameState.sleep_on_loop) platform::sleep_usec(sleep_usec);
    }
  }

  return 0;
}
