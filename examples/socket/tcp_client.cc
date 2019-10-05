#include <gflags/gflags.h>

#include "network/network.h"

DEFINE_string(hostname, "127.0.0.1", "hostname");
DEFINE_string(port, "8080", "port");
DEFINE_string(message, "Hello!", "message to send");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (!network::SocketInit()) {
    printf("Failed to initialize...\n\n");
    return 1;
  }

  printf("Configuring remote address...\n\n");
  struct addrinfo hints = {0};
  hints.ai_socktype = SOCK_STREAM;  // TCP - use SOCK_DGRAM for UDP
  struct addrinfo* peer_address;
  printf("  getaddrinfo(%s, %s, &hints, &peer_address)\n\n",
         FLAGS_hostname.c_str(), FLAGS_port.c_str());
  if (getaddrinfo(FLAGS_hostname.c_str(), FLAGS_port.c_str(),
                  &hints, &peer_address)) {
    fprintf(stderr, "getaddrinfo() failed (%d)\n\n",
            network::SocketErrno());
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

  getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
              address_buffer, sizeof(address_buffer),
              service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
  printf("%s %s\n\n", address_buffer, service_buffer);

  printf("Creating socket...\n\n");
  SOCKET socket_peer;
  printf(
      "  socket_peer = socket("
      "peer_address->ai_family, peer_address->ai_socktype,"
      " peer_address->ai_protocol)\n\n");

  socket_peer = socket(
      peer_address->ai_family, peer_address->ai_socktype,
      peer_address->ai_protocol);
  if (!network::SocketIsValid(socket_peer)) {
    fprintf(stderr, "socket() failed (%d)\n\n",
            network::SocketErrno());
    return 1;
  }

  printf("Connecting...\n\n");
  printf("  connect(socket_peer, peer_address->ai_addr,"
         " peer_address->ai_addrlen)\n\n");
  if (connect(socket_peer, peer_address->ai_addr,
              peer_address->ai_addrlen)) {
    fprintf(stderr, "connect() failed (%d)\n\n",
            network::SocketErrno());
    return 1;
  }

  freeaddrinfo(peer_address);

  printf("Connected...\n\n");

  printf("Sending: %s\n\n", FLAGS_message.c_str());
  printf("  send(socket_peer, FLAGS_message.c_str(),"
         " FLAGS_message.size(), 0)\n\n");

  int bytes_sent = send(socket_peer, FLAGS_message.c_str(),
                        FLAGS_message.size(), 0);

  printf("Sent %d bytes.\n\n", bytes_sent);

  char read[4096];
  printf("  bytes_received = recv(socket_peer, read, 4096, 0)\n\n");
  int bytes_received = recv(socket_peer, read, 4096, 0);
  if (bytes_received < 1) {
    fprintf(stderr, "Connection closed by peer...\n\n");
    return 1;
  }
  printf("Received (%d bytes): %.*s\n\n",
         bytes_received, bytes_received, read);

  return 0;
}
