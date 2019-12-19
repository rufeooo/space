#include <gflags/gflags.h>

#include "network/network.h"

DEFINE_string(hostname, "127.0.0.1", "hostname");
DEFINE_string(port, "9843", "port");
DEFINE_string(message, "Hello!", "message to send");

int
main(int argc, char** argv)
{
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (!network::SocketInit()) {
    printf("Failed to initialize...\n\n");
    return 1;
  }

  printf("Configuring remote address...\n\n");
  struct addrinfo hints = {0};
  hints.ai_socktype = SOCK_DGRAM;  // UDP - use SOCK_STREAM for UDP
  struct addrinfo* peer_address;
  printf("  getaddrinfo(%s, %s, &hints, &peer_address)\n\n",
         FLAGS_hostname.c_str(), FLAGS_port.c_str());
  if (getaddrinfo(FLAGS_hostname.c_str(), FLAGS_port.c_str(), &hints,
                  &peer_address)) {
    fprintf(stderr, "getaddrinfo() failed (%d)\n\n", network::SocketErrno());
    return 1;
  }

  printf("Remote address is...\n\n");
  char address_buffer[100];
  char service_buffer[100];
  printf(
      "  getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,"
      " address_buffer, sizeof(address_buffer),"
      " service_buffer, sizeof(service_buffer),"
      " NI_NUMERICHOST)\n\n");

  getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer,
              sizeof(address_buffer), service_buffer, sizeof(service_buffer),
              NI_NUMERICHOST);
  printf("%s %s\n\n", address_buffer, service_buffer);

  printf("Creating socket...\n\n");
  SOCKET socket_peer;
  printf(
      "  socket_peer = socket("
      "peer_address->ai_family, peer_address->ai_socktype,"
      " peer_address->ai_protocol)\n\n");

  socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype,
                       peer_address->ai_protocol);
  if (!network::SocketIsValid(socket_peer)) {
    fprintf(stderr, "socket() failed (%d)\n\n", network::SocketErrno());
    return 1;
  }

  printf("Sending: %s\n\n", FLAGS_message.c_str());
  int bytes_sent =
      sendto(socket_peer, FLAGS_message.c_str(), FLAGS_message.size(), 0,
             peer_address->ai_addr, peer_address->ai_addrlen);

  freeaddrinfo(peer_address);
  network::SocketClose(socket_peer);

  printf("Finished...\n\n");

  return 0;
}
