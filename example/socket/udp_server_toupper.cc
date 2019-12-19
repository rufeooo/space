#include <gflags/gflags.h>

#include "network/network.cc"

#include <cstring>

int
main(int argc, char** argv)
{
  if (!network::SocketInit()) {
    printf("Failed to initialize...\n\n");
    return 1;
  }

  printf("Configuring local address...\n\n");
  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;  // UDP - use SOCK_STREAM for UDP
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo* bind_address;
  getaddrinfo(0, "8080", &hints, &bind_address);

  printf("Creating socket...\n\n");

  SOCKET socket_listen =
      socket(bind_address->ai_family, bind_address->ai_socktype,
             bind_address->ai_protocol);

  if (!network::SocketIsValid(socket_listen)) {
    fprintf(stderr, "socket() failed. (%d)\n", network::SocketErrno());
    return 1;
  }

  printf("Binding socket to local address...\n\n");
  if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
    fprintf(stderr, "bind() failed. (%d)\n", network::SocketErrno());
    return 1;
  }

  freeaddrinfo(bind_address);

  fd_set master;
  FD_ZERO(&master);
  FD_SET(socket_listen, &master);
  SOCKET max_socket = socket_listen;

  printf("Waiting for connections...\n\n");

  while (1) {
    fd_set reads;
    reads = master;
    if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
      fprintf(stderr, "select() failed. (%d)\n\n", network::SocketErrno());
      return 1;
    }
    if (!FD_ISSET(socket_listen, &reads)) continue;
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    char read[1024];
    memset(&read[0], 0, sizeof(read));
    int bytes_received =
        recvfrom(socket_listen, read, 1024, 0,
                 (struct sockaddr*)&client_address, &client_len);
    if (bytes_received < 1) {
      fprintf(stderr, "connection closed. (%d)\n\n", network::SocketErrno());
      return 1;
    }
    char address_buffer[100];
    getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer,
                sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    printf("New connection from %s\n\n", address_buffer);

    int j;
    for (j = 0; j < bytes_received; ++j) {
      read[j] = toupper(read[j]);
    }
    printf("Sending '%s' to %s\n\n", read, address_buffer);
    sendto(socket_listen, read, bytes_received, 0,
           (struct sockaddr*)&client_address, client_len);
  }

  printf("Closing listening socket...\n\n");
  network::SocketClose(socket_listen);

  return 0;
}
