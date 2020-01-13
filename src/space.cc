#include <cassert>
#include <cstdio>

#include "math/math.cc"

#include "gfx/gfx.cc"
#include "network/network.cc"
#include "simulation/camera.cc"
#include "simulation/command.cc"
#include "simulation/simulation.cc"

struct State {
  // Game and render updates per second
  uint64_t framerate = 60;
  // Calculated available microseconds per game_update
  uint64_t frame_target_usec;
  // Game clock state
  Clock_t game_clock;
  // Time it took to run a frame.
  uint64_t frame_time_usec = 0;
  // Allow yielding idle cycles to kernel
  bool sleep_on_loop = true;
  // Number of times the game has been updated.
  uint64_t game_updates = 0;
  uint64_t logic_updates = 0;
  // Number of times the game frame was exceptionally delayed
  uint64_t game_jerk = 0;
  // TODO (AN): Find a home in simulation/
  Camera player_camera[MAX_PLAYER];
};

static State kGameState;

// TODO (AN): Revisit cameras
Camera*
GetLocalCamera()
{
  return &kGameState.player_camera[kNetworkState.player_id];
}

void
ProcessWindowInput(InputBuffer* input_buffer)
{
  uint64_t event_count = 0;
  while (event_count < MAX_TICK_EVENTS) {
    PlatformEvent pevent;
    if (!window::PollEvent(&pevent)) break;

    uint64_t type = pevent.type;
    switch (type) {
      case MOUSE_DOWN:
      case MOUSE_UP:
      case KEY_DOWN:
      case KEY_UP:
        input_buffer->input_event[event_count] = pevent;
        ++event_count;
        break;
    }
  }

  input_buffer->used_input_event = event_count;
}

void
ProcessInput()
{
  InputBuffer* buffer = GetNextInputBuffer();
#ifndef HEADLESS
  ProcessWindowInput(buffer);
#else
  buffer->event_count = 0;
#endif
}

void
SimulationEvent(const PlatformEvent* event, const Camera* camera,
                math::Vec2f* translation)
{
  switch (event->type) {
    case MOUSE_DOWN: {
      if (event->button == BUTTON_LEFT) {
        math::Vec2f pos = camera::ScreenToWorldSpace(camera, event->position);
        Command* command = NewCommand();
        command->type = Command::kMove;
        command->destination =
            camera::ScreenToWorldSpace(camera, event->position);
      }
    } break;
    case KEY_DOWN: {
      switch (event->key) {
        case 'w': {
          translation->y = 1.f;
        } break;
        case 'a': {
          translation->x = -1.f;
        } break;
        case 's': {
          translation->y = -1.f;
        } break;
        case 'd': {
          translation->x = 1.f;
        } break;
        default:
          break;
      }
    } break;
    case KEY_UP: {
      switch (event->key) {
        case 'w': {
          translation->y = 0.f;
        } break;
        case 'a': {
          translation->x = 0.f;
        } break;
        case 's': {
          translation->y = 0.f;
        } break;
        case 'd': {
          translation->x = 0.f;
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
ProcessSimulation(int player_id, uint64_t event_count,
                  const PlatformEvent* event)
{
  // Shared player control of the ship for now
  for (int i = 0; i < event_count; ++i) {
    SimulationEvent(&event[i], &kGameState.player_camera[player_id],
                    &kGameState.player_camera[player_id].translation);
  }
}

int
main(int argc, char** argv)
{
  while (1) {
    int opt = platform_getopt(argc, argv, "i:p:n:");
    if (opt == -1) break;

    switch (opt) {
      case 'i':
        kNetworkState.server_ip = platform_optarg;
        break;
      case 'p':
        kNetworkState.server_port = platform_optarg;
        break;
      case 'n':
        kNetworkState.num_players = strtol(platform_optarg, NULL, 10);
        break;
    }
  }
  printf("Client will connect to game at %s:%s\n", kNetworkState.server_ip,
         kNetworkState.server_port);

#ifndef HEADLESS
  // Platform & Gfx init
  if (!gfx::Initialize()) {
    return 1;
  }
#endif

  // Camera init
  // TODO (AN): don't assume every camera uses local window size
#ifndef HEADLESS
  math::Vec2f dims = window::GetWindowSize();
#else
  math::Vec2f dims = {1280.0f, 720.0f};
#endif
  for (int i = 0; i < MAX_PLAYER; ++i) {
    camera::InitialCamera(&kGameState.player_camera[i], dims);
  }
  const Camera* cam = GetLocalCamera();
  rgg::SetProjectionMatrix(cam->projection);
  rgg::SetViewMatrix(camera::view_matrix(cam));
  rgg::SetCameraTransformMatrix(camera::transform_matrix(cam));

  // Game init
  if (!simulation::Initialize()) {
    return 1;
  }

  // Network handshake uses a clock
  if (!NetworkSetup()) {
    return 1;
  }

  // Reset State
  kGameState.game_updates = 0;
  kGameState.game_jerk = 0;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;

#ifndef HEADLESS
  // If vsync is enabled, force the clock_init to align with clock_sync
  // TODO: We should also enforce framerate is equal to refresh rate
  window::SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
  // Reset the clock for simulation
  platform::clock_init(kGameState.frame_target_usec, &kGameState.game_clock);
  while (!window::ShouldClose()) {
    ProcessInput();
    NetworkEgress();
    NetworkIngress(kGameState.logic_updates);

    uint64_t slot = NETQUEUE_SLOT(kGameState.logic_updates);
    if (SlotReady(slot)) {
      // Verify the simulation has not changed outside this block
      if (!simulation::VerifyIntegrity()) exit(4);

      // Game Mutation: Apply player commands for turn N
      InputBuffer* game_turn = GetSlot(slot);
      for (int i = 0; i < MAX_PLAYER; ++i) {
        InputBuffer* player_turn = &game_turn[i];
        ProcessSimulation(i, player_turn->used_input_event,
                          player_turn->input_event);
      }

      // Game Mutation: continue simulation
      simulation::Update();

      // Camera
      for (int i = 0; i < MAX_PLAYER; ++i) {
        kGameState.player_camera[i].position +=
            kGameState.player_camera[i].translation;
      }
      const Camera* cam = GetLocalCamera();
      rgg::SetProjectionMatrix(cam->projection);
      rgg::SetViewMatrix(camera::view_matrix(cam));
      rgg::SetCameraTransformMatrix(camera::transform_matrix(cam));

      // Give the user an update tick. The engine runs with
      // a fixed delta so no need to provide a delta time.
      ++kGameState.logic_updates;
    }

    // Misc debug/feedback
    auto sz = window::GetWindowSize();
    char buffer[50];
    sprintf(buffer, "Frame Time:%06lu us", kGameState.frame_time_usec);
    gfx::PushText(buffer, 3.f, sz.y);
    sprintf(buffer, "Window Size:%ix%i", (int)sz.x, (int)sz.y);
    gfx::PushText(buffer, 3.f, sz.y - 25.f);
    auto mouse = camera::ScreenToWorldSpace(GetLocalCamera(),
                                            window::GetCursorPosition());
    sprintf(buffer, "Mouse Pos In World:(%.1f,%.1f)", mouse.x, mouse.y);
    gfx::PushText(buffer, 3.f, sz.y - 50.f);

    for (int i = 0; i < kCountAsteroid; ++i) {
      math::AxisAlignedRect aabb = gfx::kGfx.asteroid_aabb;
      aabb.min += kAsteroid[i].transform.position;
      aabb.max += kAsteroid[i].transform.position;
      if (math::PointInRect(mouse, aabb)) {
        gfx::PushText("Mouse / Asteroid collision", 3.f, sz.y - 75.f);
      }
    }

#ifndef HEADLESS
    gfx::Render();
#endif

    // Capture frame time before the potential stall on vertical sync
    kGameState.frame_time_usec = platform::delta_usec(&kGameState.game_clock);

#ifndef HEADLESS
    window::SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

#if 0
    printf("[frame %lu]\n", kGameState.game_updates);
#endif
    ++kGameState.game_updates;

    uint64_t sleep_usec = 0;
    uint64_t sleep_count = kGameState.sleep_on_loop;
    while (!platform::clock_sync(&kGameState.game_clock, &sleep_usec)) {
      while (sleep_count) {
        --sleep_count;
        platform::sleep_usec(sleep_usec);
      };
    }
  }

  return 0;
}
