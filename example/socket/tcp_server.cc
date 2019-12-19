#include <gflags/gflags.h>

#include "network/network.cc"

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
  hints.ai_socktype = SOCK_STREAM;  // TCP - use SOCK_DGRAM for UDP
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
  printf("Listening...\n\n");
  if (listen(socket_listen, 10) < 0) {
    fprintf(stderr, "listen() failed. (%d)\n\n", network::SocketErrno());
    return 1;
  }

  fd_set master;
  FD_ZERO(&master);
  FD_SET(socket_listen, &master);
  SOCKET max_socket = socket_listen;

  printf("Waiting for connections...\n\n");

  while (1) {
    fd_set reads;
    reads = master;
    if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
      fprintf(stderr, "select() failed. (%d)\n", network::SocketErrno());
      return 1;
    }
    SOCKET i;
    for (i = 1; i <= max_socket; ++i) {
      // FD_ISSET only true for sockets that area ready to be read
      if (!FD_ISSET(i, &reads)) continue;
      if (i == socket_listen) {
        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);
        SOCKET socket_client = accept(
            socket_listen, (struct sockaddr*)&client_address, &client_len);
        if (!network::SocketIsValid(socket_client)) {
          fprintf(stderr, "accept() failed. (%d)\n", network::SocketErrno());
          return 1;
        }
        FD_SET(socket_client, &master);
        if (socket_client > max_socket) max_socket = socket_client;
        char address_buffer[100];
        getnameinfo((struct sockaddr*)&client_address, client_len,
                    address_buffer, sizeof(address_buffer), 0, 0,
                    NI_NUMERICHOST);
        printf("New connection from %s\n\n", address_buffer);
      } else {
        char read[1024];
        int bytes_received = recv(i, read, 1024, 0);
        if (bytes_received < 1) {
          FD_CLR(i, &master);
          network::SocketClose(i);
          continue;
        }
        int j;
        for (j = 0; j < bytes_received; ++j) {
          read[j] = toupper(read[j]);
        }
        send(i, read, bytes_received, 0);
      }
    }
  }
  printf("Closing listening socket...\n\n");
  network::SocketClose(socket_listen);
  printf("Finished...\n\n");
  return 0;
}
