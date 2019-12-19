#include <stdio.h>
#include <string.h>
#include <time.h>

#include "network/network.cc"

int
main()
{
  network::SocketInit();

  printf("Configuring local address...\n\n");
  struct addrinfo hints = {0};
  // memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;        // AF_INET=IPv4, AF_INET6=IPv6
  hints.ai_socktype = SOCK_STREAM;  // SOCK_STREAM=TCP, SOCK_DGRAM=UDP
  hints.ai_flags = AI_PASSIVE;      // AI_PASSIVE=wildcard address,
                                    //   listen on any available net
  printf("hints.ai_flags=%i(AF_INET)\n", hints.ai_flags);
  printf("hints.ai_family=%i(SOCK_STREAM)\n", hints.ai_family);
  printf("hints.ai_socktype=%i(AI_PASSIVE)\n\n", hints.ai_socktype);

  // Make an address suitable for bind()
  struct addrinfo *bind_address;
  // Protocol independent. Makes it easy to convert from IPv4 to IPv6.
  // http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
  printf("  getaddrinfo(hints, &bind_address)\n\n");
  getaddrinfo(0, "8080", &hints, &bind_address);

  printf("bind_address.ai_flags=%i\n", bind_address->ai_flags);
  printf("bind_address.ai_family=%i\n", bind_address->ai_family);
  printf("bind_address.ai_socktype=%i\n", bind_address->ai_socktype);
  printf("bind_address.ai_protocol=%i\n", bind_address->ai_protocol);
  printf("bind_address.ai_addrlen=%i\n", bind_address->ai_addrlen);
  printf("bind_address.ai_addr.sa_family=%i\n",
         bind_address->ai_addr->sa_family);
  printf("bind_address.ai_addr.sa_data=%s\n", bind_address->ai_addr->sa_data);
  printf("bind_address.ai_canonname=%s\n\n", bind_address->ai_canonname);

  printf("Creating socket...\n\n");

  printf(
      "  socket_listen = socket(bind_address->ai_family, "
      "bind_address->ai_socktype, bind_address->ai_protocol)\n\n");

  SOCKET socket_listen =
      socket(bind_address->ai_family, bind_address->ai_socktype,
             bind_address->ai_protocol);

  if (!network::SocketIsValid(socket_listen)) {
    fprintf(stderr, "socket() failed. (%d)\n", network::SocketErrno());
    return 1;
  }

  printf("socket_listen(%i) is valid...\n\n", socket_listen);

  printf("Binding socket to local address...\n\n");

  printf(
      "  bind(socket_listen, bind_address->ai_addr, "
      "bind_address->ai_addrlen)\n\n");

  if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
    fprintf(stderr, "bind() failed. (%d)\n", network::SocketErrno());
    return 1;
  }

  printf("bind() succeeded...\n\n");

  freeaddrinfo(bind_address);

  printf("Listening...\n\n");

  printf("  listen(socket_listen, 10)\n\n");

  // Listen  10 is how many connections it is allowed to queue up.
  if (listen(socket_listen, 10) < 0) {
    fprintf(stderr, "listen() failed. (%d)\n", network::SocketErrno());
    return 1;
  }

  printf("listen() succeeded...\n\n");

  printf("Waiting for connection...\n\n");

  struct sockaddr_storage client_address;
  socklen_t client_len = sizeof(client_address);
  printf(
      "  socket_client = accept(socket_listen, (struct sockaddr*) "
      "&client_address,  &client_len)\n\n");
  SOCKET socket_client =
      accept(socket_listen, (struct sockaddr *)&client_address, &client_len);
  if (!network::SocketIsValid(socket_client)) {
    fprintf(stderr, "accept() failed. (%d)\n", network::SocketErrno());
    return 1;
  }

  printf("Client is connected...\n\n");

  printf(
      "  getnameinfo((struct sockaddr*)&client_address, "
      "client_len, address_buffer, sizeof(address_buffer), 0, 0, "
      "NI_NUMERICHOST)\n\n");
  char address_buffer[100];
  getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer,
              sizeof(address_buffer), 0, 0, NI_NUMERICHOST);

  printf("Client %s connected...\n\n", address_buffer);

  printf("Reading request...\n\n");

  char request[1024];
  printf(
      "  bytes_received = recv(socket_client, request, 1024, "
      "0)\n\n");
  int bytes_received = recv(socket_client, request, 1024, 0);
  printf("Received %d bytes...\n\n", bytes_received);

  printf("%.*s\n", bytes_received, request);

  printf("Sending response...\n\n");
  const char *response =
      "HTTP/1.1 200 OK\r\n"
      "Connection: close\r\n"
      "Content-Type: text/plain\r\n\r\n"
      "Local time is: ";
  printf(
      "  bytes_sent = send(socket_client, response, "
      "strlen(response), 0)\n\n");
  int bytes_sent = send(socket_client, response, strlen(response), 0);
  printf("Sent %d of %d bytes.\n\n", bytes_sent, (int)strlen(response));

  time_t timer;
  time(&timer);
  char *time_msg = ctime(&timer);
  printf(
      "  bytes_sent = send(socket_client, time_msg, "
      "strlen(time_msg), 0)\n\n");

  bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
  printf("Sent %d of %d bytes.\n\n", bytes_sent, (int)strlen(time_msg));

  printf("Closing connection...\n\n");
  network::SocketClose(socket_client);

  printf("Closing listening socket...\n\n");
  network::SocketClose(socket_listen);

  // TODO: network::SocketCleanup()

  printf("Finished...\n\n");

  return 0;
}
