#include "platform.cc"

#include <cstdio>

volatile bool running = true;

int
main(int argc, char** argv)
{
#define MAX_BUFFER 4 * 1024
  uint8_t buffer[MAX_BUFFER];
  udp::Init();

  Udp4 location;
  if (!udp::GetAddr4("127.0.0.1", "5000", &location)) {
    puts("fail GetAddr4");
    exit(1);
  }

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
    printf("socket %d echo %d bytes\n", location.socket, received_bytes);
    if (!udp::SendTo(location, peer, buffer, received_bytes))
    {
      puts("send failed");
    }
    else
    {
      puts("send ok");
    }
  }
}
