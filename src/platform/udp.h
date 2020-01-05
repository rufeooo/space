#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

extern "C" {
extern int udp_errno;
}

struct Udp4 {
#ifdef _WIN32
  SOCKET socket;
#else
  // File descriptor for socket
  int socket;
#endif
  // Family, address, port of destination
  char socket_address[16];
};


#ifdef __APPLE__
// TODO: Verify this value is correct.
#define MSG_NOSIGNAL 0x4000
#endif
