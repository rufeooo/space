#include <cstdio>
#include <cstring>

#include "platform/platform.cc"

static ThreadInfo thread;

struct ServerParam {
  const char* ip;
  const char* port;
};
static ServerParam thread_param;

struct PlayerState {
  Udp4 peer;
  uint64_t num_players;
  uint64_t game_id;
};

static bool running = true;
#define MAX_BUFFER (4 * 1024)
const uint64_t greeting_size = 6;
const uint64_t greeting_packet = greeting_size + sizeof(uint64_t);
const char greeting[greeting_size] = {"space"};
#define MAX_PLAYER 2
PlayerState player[MAX_PLAYER];
uint64_t used_player = 0;
uint64_t next_game_id = 1;
bool game_ready;

int
GetPlayerIndex(Udp4* peer)
{
  for (int i = 0; i < MAX_PLAYER; ++i) {
    if (memcmp(peer, &player[i].peer, sizeof(Udp4)) == 0) return i;
  }

  return -1;
}

void*
server_main(ThreadInfo* t)
{
  ServerParam* thread_param = (ServerParam*)t->arg;

  uint8_t buffer[MAX_BUFFER];
  if (!udp::Init()) {
    puts("server: fail init");
    return 0;
  }

  Udp4 location;
  if (!udp::GetAddr4(thread_param->ip, thread_param->port, &location)) {
    puts("server: fail GetAddr4");
    puts(thread_param->ip);
    puts(thread_param->port);
    return 0;
  }

  printf("Server binding %s:%s\n", thread_param->ip, thread_param->port);
  if (!udp::Bind(location)) {
    puts("server: fail Bind");
    return 0;
  }

  while (running) {
    uint16_t received_bytes;
    Udp4 peer;

    if (!udp::ReceiveAny(location, MAX_BUFFER, buffer, &received_bytes,
                         &peer)) {
      if (udp_errno) running = false;
      if (udp_errno) printf("udp_errno %d\n", udp_errno);
      platform::sleep_usec(1000);
      continue;
    }

    int pidx = GetPlayerIndex(&peer);

    // Handshake packet
    if (received_bytes >= greeting_packet &&
        strncmp(greeting, (char*)buffer, greeting_size) == 0) {
      // No room for clients on this server
      if (used_player >= MAX_PLAYER) continue;
      // Duplicate handshake packet, idx already assigned
      if (pidx != -1) continue;

      uint64_t* header = (uint64_t*)(buffer + greeting_size);
      uint64_t num_players = *header;
      ++header;
      uint64_t player_index = used_player;
      printf("Accepted %lu\n", player_index);
      player[player_index].peer = peer;
      player[player_index].num_players = num_players;
      player[player_index].game_id = 0;
      ++used_player;

      int ready_players = 0;
      for (int i = 0; i < used_player; ++i) {
        if (player[i].game_id) continue;
        if (player[i].num_players != num_players) continue;
        ++ready_players;
      }

      if (ready_players >= num_players) {
        uint64_t* header = (uint64_t*)(buffer);
        memcpy(buffer, greeting, greeting_size);

        uint64_t player_id = 0;
        for (int i = 0; i < used_player; ++i) {
          if (player[i].game_id) continue;
          if (player[i].num_players != num_players) continue;

          printf("greet player index %d\n", i);
          header = (uint64_t*)(buffer + greeting_size);
          *header = player_id;
          ++header;
          *header = num_players;
          ++header;
          if (!udp::SendTo(location, player[i].peer, buffer,
                           greeting_size + 2 * sizeof(uint64_t)))
            puts("greet failed");
          player[player_id].game_id = next_game_id;
          ++player_id;
        }
        ++next_game_id;
      }
    }

    // Filter Identified, Game-ready clients
    if (pidx == -1) continue;
    if (!player[pidx].game_id) continue;

    // Echo bytes to game participants
    uint64_t game_id = player[pidx].game_id;
#if 0
    printf("socket %d echo %d bytes to %lu game_id\n", location.socket, received_bytes, game_id);
#endif
    for (int i = 0; i < used_player; ++i) {
      if (player[i].game_id != game_id) continue;

      if (!udp::SendTo(location, player[i].peer, buffer, received_bytes)) {
        puts("server send failed");
        break;
      }
    }
  }

  return 0;
}

bool
CreateNetworkServer(const char* ip, const char* port)
{
  if (thread.id) return false;

  thread.arg = &thread_param;
  thread_param.ip = ip;
  thread_param.port = port;
  platform::thread_create(&thread, server_main);

  return true;
}

uint64_t
WaitForNetworkServer()
{
  if (!thread.id) return 0;

  platform::thread_join(&thread);
  return thread.return_value;
}

