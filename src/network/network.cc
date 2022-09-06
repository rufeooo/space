#include <cstdio>

#include "math/math.cc"

#include "server.cc"

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

struct NetworkState {
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
  InputBuffer player_input[MAX_NETQUEUE][MAX_PLAYER];
  bool player_received[MAX_NETQUEUE][MAX_PLAYER];
  uint64_t outgoing_ack[MAX_PLAYER];
};

static NetworkState kNetworkState;

bool
NetworkSetup()
{
  // No player takes action on frame 0
  // Initialize with frame 0 "ready" for update
  for (int i = 0; i < MAX_PLAYER; ++i) {
    kNetworkState.player_received[0][i] = true;
  }

  if (!udp::Init()) return false;

  if (strcmp("localhost", kNetworkState.server_ip) == 0) {
    if (!CreateNetworkServer("localhost", "9845")) return false;
  }

  if (!udp::GetAddr4(kNetworkState.server_ip, kNetworkState.server_port,
                     &kNetworkState.socket))
    return false;

  const uint64_t greeting_size = 6;
  const char greeting[greeting_size] = {"space"};
  uint64_t jerk;
  int16_t bytes_received = 0;
  Clock_t handshake_clock;
  const uint64_t usec = 5 * 1000;
  platform::clock_init(usec, &handshake_clock);
  Handshake h = {.num_players = kNetworkState.num_players};
  for (int send_count = 0; bytes_received <= 0 && send_count < 5000000;
       ++send_count) {
    printf("Client: send %s for %lu players\n", h.greeting,
           kNetworkState.num_players);
    if (!udp::Send(kNetworkState.socket, &h, sizeof(h))) exit(1);

    for (int per_send = 0; per_send < 10; ++per_send) {
      if (udp::ReceiveFrom(kNetworkState.socket,
                           sizeof(kNetworkState.netbuffer),
                           kNetworkState.netbuffer, &bytes_received))
        break;
      uint64_t sleep_usec = 0;
      platform::clock_sync(&handshake_clock, &sleep_usec);
      platform::sleep_usec(sleep_usec);
    }
  }

  printf("Client: handshake completed %d\n", bytes_received);
  if (bytes_received != sizeof(NotifyStart)) exit(3);

  NotifyStart* ns = (NotifyStart*)kNetworkState.netbuffer;
  printf(
      "Handshake result: [ player_id %zu ] [ player_count %zu ] [ game_id %zu "
      "] \n",
      (size_t)ns->player_id, (size_t)ns->player_count, (size_t)ns->game_id);

  kNetworkState.player_id = ns->player_id;
  kNetworkState.player_count = ns->player_count;

  return true;
}

InputBuffer*
GetNextInputBuffer()
{
  uint64_t slot = NETQUEUE_SLOT(kNetworkState.outgoing_sequence);
  uint64_t event_count = 0;

#if 0
  printf("ProcessInput [ %lu seq ][ %lu slot ]\n", kNetworkState.outgoing_sequence,
         slot);
#endif

  // If unacknowledged packets exceed the queue, give up
  if (kNetworkState.outgoing_sequence -
          kNetworkState.outgoing_ack[kNetworkState.player_id] >=
      MAX_NETQUEUE)
    exit(2);

  kNetworkState.outgoing_sequence += 1;

  return &kNetworkState.input[slot];
}

InputBuffer*
GetSlot(uint64_t slot)
{
  for (int i = 0; i < MAX_PLAYER; ++i) {
    kNetworkState.player_received[slot][i] = false;
  }
  return kNetworkState.player_input[slot];
}

bool
SlotReady(uint64_t slot)
{
  for (int i = 0; i < kNetworkState.player_count; ++i) {
    if (!kNetworkState.player_received[slot][i]) return false;
  }

  return true;
}

void
NetworkSend(uint64_t seq)
{
  uint64_t slot = NETQUEUE_SLOT(seq);
  InputBuffer* ibuf = &kNetworkState.input[slot];

  // write frame
  Turn* header = (Turn*)kNetworkState.netbuffer;
  header->sequence = seq;
  header->player_id = kNetworkState.player_id;
#if 0
  printf("CliSnd [ %lu seq ] [ %lu slot ] [ %lu player_id ] [ %lu events ]\n",
         seq, slot, kNetworkState.player_id, ibuf->used_input_event);
#endif

  // write input
  memcpy(header->event, ibuf->input_event,
         sizeof(PlatformEvent) * ibuf->used_input_event);

  if (!udp::Send(
          kNetworkState.socket, kNetworkState.netbuffer,
          sizeof(Turn) + sizeof(PlatformEvent) * ibuf->used_input_event)) {
    exit(1);
  }
}

void
NetworkEgress()
{
  uint64_t begin_seq = kNetworkState.outgoing_ack[kNetworkState.player_id] + 1;
  uint64_t end_seq = kNetworkState.outgoing_sequence;

  // Re-send input history
  for (uint64_t i = begin_seq; i < end_seq; ++i) {
    NetworkSend(i);
  }
}

void
NetworkIngress(uint64_t current_frame)
{
  uint64_t local_player = kNetworkState.player_id;

  int16_t bytes_received;
  while (udp::ReceiveFrom(kNetworkState.socket, sizeof(kNetworkState.netbuffer),
                          kNetworkState.netbuffer, &bytes_received)) {
    NotifyTurn* header = (NotifyTurn*)kNetworkState.netbuffer;
    uint64_t frame = header->frame;
    uint64_t player_id = header->player_id;
#if 0
    printf("CliRcv [ %lu frame ] [ %lu player_id ] [ %lu ack_seq ]\n", frame,
           player_id, header->ack_sequence);
#endif

    // Drop old frames, the game has progressed
    if (frame < current_frame) continue;
    // Personal boundaries
    if (player_id >= MAX_PLAYER) exit(1);
    if (bytes_received > sizeof(NotifyTurn) + sizeof(InputBuffer::input_event))
      exit(3);

    uint64_t slot = NETQUEUE_SLOT(frame);
    InputBuffer* ibuf = &kNetworkState.player_input[slot][player_id];
    memcpy(ibuf->input_event, header->event,
           bytes_received - sizeof(NotifyTurn));
    ibuf->used_input_event =
        (bytes_received - sizeof(NotifyTurn)) / sizeof(PlatformEvent);
#if 0
    printf("Copied %lu, used_input_event %lu\n", bytes_received - header_size,
           ibuf->used_input_event);
#endif
    kNetworkState.player_received[slot][player_id] = true;
    // Accept highest received ack_sequence
    kNetworkState.outgoing_ack[player_id] =
        MAX(kNetworkState.outgoing_ack[player_id], header->ack_sequence);
  }
}

