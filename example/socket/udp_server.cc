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

  struct sockaddr_storage client_address;
  socklen_t client_len = sizeof(client_address);
  char read[1024];

  printf("Waiting for data...\n\n");
  int bytes_received = recvfrom(socket_listen, read, 1024, 0,
                                (struct sockaddr*)&client_address, &client_len);

  printf("Received (%d bytes): %.*s\n\n", bytes_received, bytes_received, read);

  printf("Remote address is: ");
  char address_buffer[100];
  char service_buffer[100];
  getnameinfo(((struct sockaddr*)&client_address), client_len, address_buffer,
              sizeof(address_buffer), service_buffer, sizeof(service_buffer),
              NI_NUMERICHOST | NI_NUMERICSERV);

  printf("%s %s\n\n", address_buffer, service_buffer);

  network::SocketClose(socket_listen);

  printf("Finished...\n");

  return 0;
}
