#include <pthread.h>
#include <cstdio>

#include "platform/platform.cc"

struct ServerParam {
  const char* ip;
  const char* port;
};

static pthread_attr_t attr;
static pthread_t thread;
static ServerParam param;
static bool running = true;
#define MAX_BUFFER (4 * 1024)

void*
server_main(void* arg)
{
  ServerParam* param = (ServerParam*)arg;

  uint8_t buffer[MAX_BUFFER];
  udp::Init();

  Udp4 location;
  if (!udp::GetAddr4(param->ip, param->port, &location)) {
    puts("fail GetAddr4");
    exit(1);
  }

  printf("Server binding %s:%s\n", param->ip, param->port);
  if (!udp::Bind(location)) {
    puts("fail Bind");
    exit(1);
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

    // Echo bytes to peer
#if 0
    printf("socket %d echo %d bytes\n", location.socket, received_bytes);
#endif
    if (!udp::SendTo(location, peer, buffer, received_bytes)) {
      puts("server send failed");
    }
  }

  return 0;
}

bool
CreateNetworkServer(const char* ip, const char* port)
{
  if (thread) return false;

  param.ip = ip;
  param.port = port;
  pthread_attr_init(&attr);
  pthread_create(&thread, &attr, server_main, (void*)&param);

  return true;
}
