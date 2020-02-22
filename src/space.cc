#include <cassert>
#include <cstdio>

#include "math/math.cc"

#include "gfx/gfx.cc"
#include "network/network.cc"
#include "simulation/camera.cc"
#include "simulation/interaction.cc"
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
  // (optional) limit the simulation frames (UINT64_MAX will loop infinitely)
  uint64_t frame_limit = UINT64_MAX;
  // (optional) yield unused cpu time to the system
  bool sleep_on_loop = true;
  // Number of times the game has been updated.
  uint64_t game_updates = 0;
  uint64_t logic_updates = 0;
  // Number of times the game frame was exceptionally delayed
  uint64_t game_jerk = 0;
};

static State kGameState;
// TODO (AN): Find a home in simulation/
static Camera player_camera[MAX_PLAYER];
static Stats stats;

// TODO (AN): Revisit cameras
const Camera*
GetMyCamera()
{
  return &player_camera[kNetworkState.player_id];
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
      case MOUSE_WHEEL:
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
  buffer->used_input_event = 0;
#endif
}

void
SetProjection()
{
  // TODO (AN): revisit camera assumptions
#ifdef HEADLESS
  v2f size = {1280.0f, 720.0f};
#else
  v2f size = window::GetWindowSize();
#endif
  rgg::GetObserver()->projection =
      math::Ortho(size.x, 0.f, size.y, 0.f, -100.f, 0.f);
}

v3f
MyScreenToWorld(v2f xy)
{
  return camera::ScreenToWorldSpace(GetMyCamera(),
                                    v3f(xy - window::GetWindowSize() * 0.5f));
}

void
ProcessSimulation(int player_id, uint64_t event_count,
                  const PlatformEvent* event)
{
  // Shared player control of the ship for now
  for (int i = 0; i < event_count; ++i) {
    simulation::ControlEvent(&event[i], &player_camera[player_id],
                             &player_camera[player_id].motion);
  }
}

int
main(int argc, char** argv)
{
  while (1) {
    int opt = platform_getopt(argc, argv, "i:p:n:f:s:");
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
      case 'f':
        kGameState.frame_limit = strtol(platform_optarg, NULL, 10);
        break;
      case 's':
        simulation::kScenario.type =
            (simulation::Scenario::Type)strtol(platform_optarg, NULL, 10);
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
  for (int i = 0; i < MAX_PLAYER; ++i) {
    camera::InitialCamera(&player_camera[i]);
  }
  camera::SetView(GetMyCamera(), &rgg::GetObserver()->view);

  // Projection init
  SetProjection();

  // Game init
  if (!simulation::Initialize()) {
    return 1;
  }

  // Network handshake uses a clock
  if (!NetworkSetup()) {
    return 1;
  }

  // main thread affinity set to core 0
  if (platform::thread_affinity_count() > 1) {
    platform::thread_affinity_set(0);
    printf("Game thread may run on %d cores\n",
           platform::thread_affinity_count());
  }

  uint64_t bytes = 0;
  uint64_t min_ptr = UINT64_MAX;
  uint64_t max_ptr = 0;
  for (int i = 0; i < kUsedRegistry; ++i) {
    printf("Registry ptr %p\n", kRegistry[i].ptr);
    bytes += kRegistry[i].memb_max * kRegistry[i].memb_size;
    max_ptr = MAX((uint64_t)kRegistry[i].ptr, max_ptr);
    min_ptr = MIN((uint64_t)kRegistry[i].ptr, min_ptr);
  }
  printf("[min page 0x%lx] [max page 0x%lx] [page_count %lu]\n",
         ANDN(PAGE - 1, min_ptr), ANDN(PAGE - 1, max_ptr),
         1 + ((ANDN(PAGE - 1, max_ptr) - ANDN(PAGE - 1, min_ptr)) / PAGE));

  // Reset State
  StatsInit(&stats);
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
  const uint64_t frame_limit = kGameState.frame_limit;
  for (uint64_t frame = 0; frame <= frame_limit; ++frame) {
    if (window::ShouldClose()) break;

    ProcessInput();
    NetworkEgress();
    NetworkIngress(kGameState.logic_updates);

    uint64_t slot = NETQUEUE_SLOT(kGameState.logic_updates);
    if (SlotReady(slot)) {
      // Hash the simulation state every 0th slot
      if (!slot)
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
        camera::Update(&player_camera[i]);
        player_camera[i].motion.z = 0.f;
      }
      camera::SetView(GetMyCamera(), &rgg::GetObserver()->view);

      // Give the user an update tick. The engine runs with
      // a fixed delta so no need to provide a delta time.
      ++kGameState.logic_updates;
    }

#ifndef HEADLESS
    // Misc debug/feedback
    v3f mouse = MyScreenToWorld(window::GetCursorPosition());
    const v2f dims = window::GetWindowSize();
    simulation::DebugPanel(mouse, stats, kGameState.frame_target_usec);
    simulation::LogPanel();
    simulation::Hud(dims);

    // The bottom left and top right of the screen with regards to the camera.
    v3f top_right = MyScreenToWorld(dims);
    v3f bottom_left = MyScreenToWorld({0.f, 0.f});
    gfx::Render(math::Rectf{bottom_left.xy(), top_right.xy()}, mouse.xy(),
                dims);
#endif
    imui::Reset();

    // Capture frame time before the potential stall on vertical sync
    const uint64_t elapsed_usec = platform::delta_usec(&kGameState.game_clock);
    kGameState.frame_time_usec = elapsed_usec;
    StatsAdd(elapsed_usec, &stats);

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
