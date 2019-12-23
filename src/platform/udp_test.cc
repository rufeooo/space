#include "platform.cc"

#include <cstdio>

int
main()
{
  Udp4 peer;
  if (!udp::GetAddr4("localhost", "5000", &peer)) {
    printf("fail getaddr4 %d\n", udp_errno);
    exit(1);
  }

  const char* payload = "wheeeee";
  if (!udp::Send(peer, payload, sizeof(payload))) {
    printf("fail send %d\n", udp_errno);
    exit(1);
  }

  return 0;
}
