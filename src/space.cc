#include <cassert>
#include <cstdio>

#include "math/math.cc"

#include "network/server.cc"
#include "simulation/camera.cc"
#include "simulation/command.cc"
#include "simulation/simulation.cc"

// Input events capable of being processed in one game loop
#define MAX_TICK_EVENTS 32
// Game loop inputs allowed in-flight on the network
#define MAX_NETQUEUE 128
// Convert frame id into a NETQUEUE slot
#define NETQUEUE_SLOT(sequence) (sequence % MAX_NETQUEUE)
// Players in one game
#define MAX_PLAYER 2
// System memory block: Move to platform?
#define PAGE (4 * 1024)

struct InputBuffer {
  PlatformEvent input_event[MAX_TICK_EVENTS];
  uint64_t used_input_event = 0;
};

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
  // Events handled per input game frame for NETQUEUE frames
  // History is preserved until network acknowledgement
  InputBuffer input[MAX_NETQUEUE];
  uint64_t outgoing_sequence = 1;
  // Network resources
  Udp4 socket;
  uint8_t netbuffer[PAGE];
  const char* server_ip = "localhost";
  const char* server_port = "9845";
  uint64_t num_players = 1;
  // Unique id for this game
  uint64_t player_id;
  // Total players in this game
  uint64_t player_count;
  // Per Player
  InputBuffer player_input[MAX_PLAYER][MAX_NETQUEUE];
  bool player_received[MAX_PLAYER][MAX_NETQUEUE];
  Camera player_camera[MAX_PLAYER];
  uint64_t outgoing_ack[MAX_PLAYER];
};

static State kGameState;

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

  const uint64_t greeting_size = 6;
  const char greeting[greeting_size] = {"space"};
  uint64_t jerk;
  int16_t bytes_received = 0;
  Clock_t handshake_clock;
  const uint64_t usec = 5 * 1000;
  platform::clock_init(usec, &handshake_clock);
  Handshake h = {.num_players = kGameState.num_players};
  for (int send_count = 0; bytes_received <= 0 && send_count < 500;
       ++send_count) {
    printf("Client: send %s for %lu players\n", h.greeting,
           kGameState.num_players);
    if (!udp::Send(kGameState.socket, &h, sizeof(h))) exit(1);

    for (int per_send = 0; per_send < 10; ++per_send) {
      if (udp::ReceiveFrom(kGameState.socket, sizeof(kGameState.netbuffer),
                           kGameState.netbuffer, &bytes_received))
        break;
      uint64_t sleep_usec = 0;
      platform::clock_sync(&handshake_clock, &sleep_usec);
      platform::sleep_usec(sleep_usec);
    }
  }

  printf("Client: handshake completed %d\n", bytes_received);
  if (bytes_received != sizeof(NotifyStart)) exit(3);

  NotifyStart* ns = (NotifyStart*)kGameState.netbuffer;
  printf(
      "Handshake result: [ player_id %zu ] [ player_count %zu ] [ game_id %zu "
      "] \n",
      (size_t)ns->player_id, (size_t)ns->player_count, (size_t)ns->game_id);

  kGameState.player_id = ns->player_id;
  kGameState.player_count = ns->player_count;

  return true;
}

uint64_t
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

  return event_count;
}

bool
ProcessInput()
{
  uint64_t slot = NETQUEUE_SLOT(kGameState.outgoing_sequence);
  uint64_t event_count = 0;

#if 0
  printf("ProcessInput [ %lu seq ][ %lu slot ]\n", kGameState.outgoing_sequence,
         slot);
#endif

  // If unacknowledged packets exceed the queue, give up
  if (kGameState.outgoing_sequence -
          kGameState.outgoing_ack[kGameState.player_id] >=
      MAX_NETQUEUE)
    exit(2);

#ifndef HEADLESS
  event_count = ProcessWindowInput(&kGameState.input[slot]);
#endif

  kGameState.input[slot].used_input_event = event_count;
  kGameState.outgoing_sequence += 1;

  return true;
}

bool
SlotReceived(uint64_t slot)
{
  for (int i = 0; i < kGameState.player_count; ++i) {
    if (!kGameState.player_received[i][slot]) return false;
  }

  return true;
}

void
NetworkSend(uint64_t seq)
{
  uint64_t slot = NETQUEUE_SLOT(seq);
  InputBuffer* ibuf = &kGameState.input[slot];

  // write frame
  Turn* header = (Turn*)kGameState.netbuffer;
  header->sequence = seq;
  header->player_id = kGameState.player_id;
#if 0
  printf("CliSnd [ %lu seq ] [ %lu slot ] [ %lu player_id ] [ %lu events ]\n",
         seq, slot, kGameState.player_id, ibuf->used_input_event);
#endif

  // write input
  memcpy(header->event, ibuf->input_event,
         sizeof(PlatformEvent) * ibuf->used_input_event);

  if (!udp::Send(
          kGameState.socket, kGameState.netbuffer,
          sizeof(Turn) + sizeof(PlatformEvent) * ibuf->used_input_event)) {
    exit(1);
  }
}

void
NetworkEgress()
{
  uint64_t begin_seq = kGameState.outgoing_ack[kGameState.player_id] + 1;
  uint64_t end_seq = kGameState.outgoing_sequence;

  // Re-send input history
  for (uint64_t i = begin_seq; i < end_seq; ++i) {
    NetworkSend(i);
  }
}

void
NetworkIngress()
{
  uint64_t local_player = kGameState.player_id;

  int16_t bytes_received;
  while (udp::ReceiveFrom(kGameState.socket, sizeof(kGameState.netbuffer),
                          kGameState.netbuffer, &bytes_received)) {
    NotifyTurn* header = (NotifyTurn*)kGameState.netbuffer;
    uint64_t frame = header->frame;
    uint64_t player_id = header->player_id;
#if 0
    printf("CliRcv [ %lu frame ] [ %lu player_id ] [ %lu ack_seq ]\n", frame,
           player_id, header->ack_sequence);
#endif

    // Drop old frames, the game has progressed
    if (frame < kGameState.logic_updates) continue;
    // Personal boundaries
    if (player_id >= MAX_PLAYER) exit(1);
    if (bytes_received > sizeof(NotifyTurn) + sizeof(InputBuffer::input_event))
      exit(3);

    uint64_t slot = NETQUEUE_SLOT(frame);
    InputBuffer* ibuf = &kGameState.player_input[player_id][slot];
    memcpy(ibuf->input_event, header->event,
           bytes_received - sizeof(NotifyTurn));
    ibuf->used_input_event =
        (bytes_received - sizeof(NotifyTurn)) / sizeof(PlatformEvent);
#if 0
    printf("Copied %lu, used_input_event %lu\n", bytes_received - header_size,
           ibuf->used_input_event);
#endif
    kGameState.player_received[player_id][slot] = true;
    // Accept highest received ack_sequence
    kGameState.outgoing_ack[player_id] =
        MAX(kGameState.outgoing_ack[player_id], header->ack_sequence);
  }
}

void
SimulationEvent(const PlatformEvent* event, const Camera* camera,
                math::Vec2f* translation)
{
  switch (event->type) {
    case MOUSE_DOWN: {
      if (event->button == BUTTON_LEFT) {
        command::Move move;
        move.entity_id = 0;
        move.position = camera::ScreenToWorldSpace(camera, event->position);
        command::Execute(move);
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
        kGameState.server_ip = platform_optarg;
        break;
      case 'p':
        kGameState.server_port = platform_optarg;
        break;
      case 'n':
        kGameState.num_players = strtol(platform_optarg, NULL, 10);
        break;
    }
  }
  printf("Client will connect to game at %s:%s\n", kGameState.server_ip,
         kGameState.server_port);

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
  const Camera* cam = &kGameState.player_camera[kGameState.player_id];
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
  // No player takes action on frame 0
  // Initialize with frame 0 "ready" for update
  for (int i = 0; i < MAX_PLAYER; ++i) {
    kGameState.player_received[i][0] = true;
  }

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
    NetworkIngress();

    uint64_t slot = NETQUEUE_SLOT(kGameState.logic_updates);
    if (SlotReceived(slot)) {
      // Verify the simulation has not changed outside this block
      if (!simulation::VerifyIntegrity()) exit(4);

      // Game Mutation: Apply player commands for turn N
      for (int i = 0; i < kGameState.player_count; ++i) {
        InputBuffer* ibuf = &kGameState.player_input[i][slot];
        ProcessSimulation(i, ibuf->used_input_event, ibuf->input_event);
        kGameState.player_received[i][slot] = false;
      }
      // Game Mutation: continue simulation
      simulation::Update();

      // Begin Render Mutation
      gfx::ResetRenderData();

      // Misc debug/feedback
      auto sz = window::GetWindowSize();
      char buffer[50];
      sprintf(buffer, "Frame Time:%06lu us", kGameState.frame_time_usec);
      gfx::PushText(buffer, 3.f, sz.y);
      sprintf(buffer, "Window Size:%ix%i", (int)sz.x, (int)sz.y);
      gfx::PushText(buffer, 3.f, sz.y - 25.f);
      auto mouse = camera::ScreenToWorldSpace(
          &kGameState.player_camera[kGameState.player_id],
          window::GetCursorPosition());
      sprintf(buffer, "Mouse Pos In World:(%.1f,%.1f)", mouse.x, mouse.y);
      gfx::PushText(buffer, 3.f, sz.y - 50.f);

      // Game simulation to renderer
      simulation::ToRenderer();

      // Commit new simulation state
      simulation::ApplyUpdate();

      // Camera
      for (int i = 0; i < kGameState.player_count; ++i) {
        kGameState.player_camera[i].position +=
            kGameState.player_camera[i].translation;
      }
      const Camera* cam = &kGameState.player_camera[kGameState.player_id];
      rgg::SetProjectionMatrix(cam->projection);
      rgg::SetViewMatrix(camera::view_matrix(cam));
      rgg::SetCameraTransformMatrix(camera::transform_matrix(cam));

      // Give the user an update tick. The engine runs with
      // a fixed delta so no need to provide a delta time.
      ++kGameState.logic_updates;
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
