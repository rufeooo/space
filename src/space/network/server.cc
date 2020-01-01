#include <cstdio>
#include <cstring>

#include "platform/platform.cc"

static ThreadInfo thread;

struct ServerParam {
  const char* ip;
  const char* port;
  uint64_t player_count;
};
static ServerParam thread_param;

static bool running = true;
#define MAX_BUFFER (4 * 1024)
const uint64_t greeting_size = 6;
const char greeting[greeting_size] = {"space"};
#define MAX_PLAYER 2
Udp4 player[MAX_PLAYER];
uint64_t used_player = 0;
bool game_ready;

int
GetPlayerId(Udp4* peer)
{
  for (int i = 0; i < MAX_PLAYER; ++i) {
    if (memcmp(peer, &player[i], sizeof(Udp4)) == 0) return i;
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

    int pid = GetPlayerId(&peer);

    if (pid == -1) {
      if (used_player >= thread_param->player_count) continue;

      if (received_bytes >= greeting_size &&
          strncmp(greeting, (char*)buffer, greeting_size) == 0) {
        uint64_t player_id = used_player;
        printf("Accepted %lu\n", player_id);
        player[player_id] = peer;
        ++used_player;

        if (used_player == thread_param->player_count) {
          uint64_t* header = (uint64_t*)(buffer);
          memcpy(buffer, greeting, greeting_size);

          for (int i = 0; i < used_player; ++i) {
            printf("greet player %d\n", i);
            header = (uint64_t*)(buffer + greeting_size);
            *header = i;
            ++header;
            *header = thread_param->player_count;
            ++header;
            if (!udp::SendTo(location, player[i], buffer,
                             greeting_size + 2 * sizeof(uint64_t)))
              puts("greet failed");
          }

          game_ready = true;
        }
      }

      continue;
    }

    // Ignore extra handshakes
    if (received_bytes >= greeting_size &&
        strncmp(greeting, (char*)buffer, greeting_size) == 0) {
      continue;
    }

    if (!game_ready) continue;

      // Echo bytes to all players
#if 0
    printf("socket %d echo %d bytes to %lu players\n", location.socket, received_bytes, thread_param->player_count);
#endif
    for (int i = 0; i < thread_param->player_count; ++i) {
      if (!udp::SendTo(location, player[i], buffer, received_bytes)) {
        puts("server send failed");
        break;
      }
    }
  }

  return 0;
}

bool
CreateNetworkServer(const char* ip, const char* port, const char* num_players)
{
  if (thread.id) return false;

  thread.arg = &thread_param;
  thread_param.ip = ip;
  thread_param.port = port;
  thread_param.player_count = atoi(num_players);
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

