#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>

#include "udp.h"

extern "C" {
int udp_errno;
}

namespace udp
{
bool
Init()
{
  // derp: windows api compat
  return true;
}

bool
Send(Udp4 peer, const void* buffer, uint16_t len)
{
  ssize_t bytes = sendto(peer.socket, buffer, len, MSG_NOSIGNAL | MSG_DONTWAIT,
                         (const struct sockaddr*)peer.socket_address,
                         sizeof(Udp4::socket_address));
  return bytes == len;
}

bool
Receive(Udp4 peer, uint16_t buffer_len, uint8_t* buffer,
        uint16_t* bytes_received)
{
  sockaddr_in remote_addr;
  socklen_t remote_len;

  do {
    ssize_t bytes = recvfrom(peer.socket, buffer, buffer_len, MSG_DONTWAIT,
                             (struct sockaddr*)&remote_addr, &remote_len);
    *bytes_received = bytes;
    if (bytes < 0) {
      udp_errno = (errno == EAGAIN) ? 0 : errno;
      return false;
    }

    if (remote_len != sizeof(Udp4::socket_address)) return false;

    // filter packets from unexpected peers
  } while (memcmp(&remote_addr, peer.socket_address,
                  sizeof(Udp4::socket_address)) != 0);

  return true;
}

bool
GetAddr4(const char* host, const char* service_or_port, Udp4* out)
{
  static struct addrinfo hints;
  struct addrinfo* result = NULL;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = 0;
  hints.ai_protocol = IPPROTO_UDP;

  if (getaddrinfo(host, service_or_port, &hints, &result) != 0) {
    udp_errno = errno;
    out->socket = -1;
    return false;
  }

  out->socket =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);

  udp_errno = errno;
  if (out->socket == -1) return false;

  if (result->ai_addrlen > sizeof(Udp4::socket_address)) return false;

  memcpy(out->socket_address, result->ai_addr, result->ai_addrlen);

  freeaddrinfo(result);

  return true;
}
}  // namespace udp
