#include <cassert>
#include <cstdio>

#include "math/math.cc"

#include "space/camera.cc"
#include "space/command.cc"
#include "space/ecs.cc"
#include "space/gfx.cc"
#include "space/network/server.cc"

#define MAX_TICK_EVENTS 30
#define MAX_PLAYER 2
#define PAGE (4 * 1024)

struct InputBuffer {
  PlatformEvent ievent[MAX_TICK_EVENTS];
  uint64_t used_ievent = 0;
};

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
  InputBuffer local_input;
  // Network resources
  Udp4 socket;
  uint8_t netbuffer[PAGE];
  const char* server_ip = "localhost";
  const char* server_port = "9845";
  // Unique id for this game
  uint64_t player_id;
  // Total players in this game
  uint64_t player_count;
  // Per Player
  InputBuffer player_input[MAX_PLAYER];
  bool player_received[MAX_PLAYER];
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
NetworkSetup()
{
  if (!udp::Init()) return false;

  if (strcmp("localhost", kGameState.server_ip) == 0) {
    if (!CreateNetworkServer("localhost", "9845")) return false;
  }

  if (!udp::GetAddr4(kGameState.server_ip, kGameState.server_port,
                     &kGameState.socket))
    return false;

  // TODO: Handshake for player_id, player_count
  kGameState.player_id = 0;
  kGameState.player_count = 1;

  // fake handshake
  platform::sleep_ms(100);

  return true;
}

void
ProcessLocalInput()
{
  InputBuffer* ibuf = &kGameState.local_input;
  ibuf->used_ievent = 0;
  while (ibuf->used_ievent < MAX_TICK_EVENTS) {
    PlatformEvent pevent;
    if (!window::PollEvent(&pevent)) break;

    uint64_t type = pevent.type;
    switch (type) {
      case MOUSE_DOWN:
      case MOUSE_UP:
      case KEY_DOWN:
      case KEY_UP:
        ibuf->ievent[ibuf->used_ievent] = pevent;
        ++ibuf->used_ievent;
        break;
    }
  }
}

void
ResetPlayersReceived()
{
  for (int i = 0; i < kGameState.player_count; ++i) {
    kGameState.player_received[i] = false;
  }
}

bool
AllPlayersReceived()
{
  for (int i = 0; i < kGameState.player_count; ++i) {
    if (!kGameState.player_received[i]) return false;
  }

  return true;
}

void
NetworkEgress()
{
  InputBuffer* ibuf = &kGameState.local_input;

  // write frame
  uint64_t* header = (uint64_t*)kGameState.netbuffer;
  *header = kGameState.game_updates;
  ++header;
  *header = kGameState.player_id;
  ++header;
  uint64_t header_size = (uint8_t*)header - kGameState.netbuffer;
#if 0
  printf("Send %lu frame %lu player_id\n", kGameState.game_updates,
         kGameState.player_id);
#endif

  // write input
  memcpy(kGameState.netbuffer + header_size, ibuf->ievent,
         sizeof(PlatformEvent) * ibuf->used_ievent);

  if (!udp::Send(kGameState.socket, kGameState.netbuffer,
                 header_size + sizeof(PlatformEvent) * ibuf->used_ievent)) {
    exit(1);
  }
}

void
NetworkIngress()
{
  uint16_t bytes_received;
  while (udp::ReceiveFrom(kGameState.socket, sizeof(kGameState.netbuffer),
                          kGameState.netbuffer, &bytes_received)) {
    // TODO: packet reordering based on frame
    uint64_t* header = (uint64_t*)kGameState.netbuffer;
    uint64_t frame = *header;
    ++header;
    uint64_t player_id = *header;
    ++header;
    uint64_t header_size = (uint8_t*)header - kGameState.netbuffer;
#if 0
    printf("%lu frame %lu player_id\n", frame, player_id);
#endif

    // sanity checks
    if (player_id >= MAX_PLAYER) exit(1);
    if (bytes_received > header_size + sizeof(InputBuffer::ievent)) exit(1);

    // TODO: queue per player
    InputBuffer* ibuf = &kGameState.player_input[player_id];
    memcpy(ibuf->ievent, kGameState.netbuffer + header_size,
           bytes_received - header_size);
    ibuf->used_ievent = (bytes_received - header_size) / sizeof(PlatformEvent);
#if 0
    printf("Copied %lu, used_ievent %lu\n", bytes_received - header_size,
           ibuf->used_ievent);
#endif
    kGameState.player_received[player_id] = true;
  }
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
ProcessGameInput(int player_id, uint64_t event_count, PlatformEvent* event)
{
  static math::Vec2f camera_translate(0.f, 0.f);
  // Shared player control of the ship for now
  for (int i = 0; i < event_count; ++i) {
    GameInput(&event[i], &camera_translate);
  }

  if (player_id != kGameState.player_id) return;

  // TODO: ecs data segregation
  // Player 0 ecs != Player 1 ecs 
  //
  // Local player camera control
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

int
main(int argc, char** argv)
{
  while (1) {
    int opt = platform_getopt(argc, argv, "i:p:");
    if (opt == -1) break;

    switch (opt) {
      case 'i':
        kGameState.server_ip = platform_optarg;
        break;
      case 'p':
        kGameState.server_port = platform_optarg;
        break;
    }
  }
  printf("Client will connect to game at %s:%s\n", kGameState.server_ip,
         kGameState.server_port);

  if (!Initialize()) {
    return 1;
  }

  if (!NetworkSetup()) {
    return 1;
  }

  kGameState.game_updates = 0;
  kGameState.game_jerk = 0;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;
  platform::clock_init();

  while (!window::ShouldClose()) {
    ProcessLocalInput();
    NetworkEgress();
    // TEMP: until frame slotting packets is done, we must lock-step 1 frame at
    // a time
    ResetPlayersReceived();
    while (!AllPlayersReceived()) {
      NetworkIngress();
      platform::sleep_ms(1000);
    }
    for (int i = 0; i < kGameState.player_count; ++i) {
      InputBuffer* ibuf = &kGameState.player_input[i];
      ProcessGameInput(i, ibuf->used_ievent, ibuf->ievent);
    }

    // Give the user an update tick. The engine runs with
    // a fixed delta so no need to provide a delta time.
    UpdateGame();
    gfx::Render();

    ++kGameState.game_updates;

    uint64_t sleep_usec = 0;
    while (!platform::elapse_usec(kGameState.frame_target_usec, &sleep_usec,
                                  &kGameState.game_jerk)) {
      if (kGameState.sleep_on_loop) platform::sleep_usec(sleep_usec);
    }
  }

  return 0;
}
