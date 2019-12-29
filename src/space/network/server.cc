#include <cstdio>
#include <cstring>

#include "platform/platform.cc"

static ThreadInfo thread;

struct ServerParam {
  const char* ip;
  const char* port;
};
static ServerParam thread_param;

static bool running = true;
#define MAX_BUFFER (4 * 1024)

void*
server_main(ThreadInfo* t)
{
  ServerParam* thread_param = (ServerParam*)t->arg;

  uint8_t buffer[MAX_BUFFER];
  uint64_t player_count = 0;
  if (!udp::Init()) {
    puts("server: fail init");
    return 0;
  }

  Udp4 location;
  if (!udp::GetAddr4(thread_param->ip, thread_param->port, &location)) {
    puts("server: fail GetAddr4");
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
      continue;
    }

    const uint64_t greeting_size = 6;
    const char greeting[greeting_size] = {"space"};
    if (received_bytes >= greeting_size &&
        strncmp(greeting, (char*)buffer, greeting_size) == 0) {
      uint64_t* header = (uint64_t*)(buffer + greeting_size);
      uint64_t player_id = player_count;
      ++player_count;
      *header = player_id;
      ++header;
      *header = player_count;
      ++header;
      if (!udp::SendTo(location, peer, buffer,
                       greeting_size + 2 * sizeof(uint64_t)))
        puts("server handshake failed");
      continue;
    }

    // Echo bytes to peer
#if 0
    printf("socket %d echo %d bytes\n", location.socket, received_bytes);
#endif
    if (!udp::SendTo(location, peer, buffer, received_bytes)) {
      puts("server send failed");
      break;
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
