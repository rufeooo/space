#include "server.h"

#include <thread>

#include "network.h"

namespace network {

namespace server {

namespace {

constexpr int kAddressSize = 100;

struct ClientConnection {
  struct sockaddr_storage client_address = {};
  socklen_t client_len = 0;
  char address_buffer[kAddressSize] = {};
};

// Lets keep it simple for now and just allow 10 clients. To receive
// messages back from us.
static struct ClientConnection kClients[10]; 
static int kClientCount = 0;

bool ClientExists(char* address) {
  for (int i = 0; i < kClientCount; ++i) {
    if (strcmp(kClients[i].address_buffer, address) == 0) return true;
  }
  return false;
}

void SendToAllClients(SOCKET socket_listen, Message msg) {
  for (int i = 0; i < kClientCount; ++i) {
    printf("Sending msg to %s data: %.*s\n\n",
           kClients[i].address_buffer, msg.size, msg.data);
    sendto(socket_listen, msg.data, msg.size, 0,
           (struct sockaddr*) &kClients[i].client_address,
           kClients[i].client_len);
    free(msg.data);  // We own this, so free it.
  }
}

void StartServer(const char* port,
                 MessageQueue* outgoing_message_queue,
                 MessageQueue* incoming_message_queue) {
  if (!SocketInit()) {
    printf("Failed to initialize...\n\n");
    return;
  }

  printf("Configuring local address...\n\n");
  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;  // UDP - use SOCK_STREAM for UDP
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo* bind_address;
  getaddrinfo(0, port, &hints, &bind_address);

  printf("Creating socket...\n\n");

  SOCKET socket_listen = socket(
      bind_address->ai_family, bind_address->ai_socktype,
      bind_address->ai_protocol);

  if (!SocketIsValid(socket_listen)) {
    fprintf(stderr, "socket() failed. (%d)\n", network::SocketErrno());
    return;
  }

  printf("Binding socket to local address...\n\n");
  if (bind(socket_listen, bind_address->ai_addr,
           bind_address->ai_addrlen)) {
    fprintf(stderr, "bind() failed. (%d)\n", network::SocketErrno());
    return;
  }

  freeaddrinfo(bind_address);

  fd_set master;
  FD_ZERO(&master);
  FD_SET(socket_listen, &master);
  SOCKET max_socket = socket_listen;

  printf("Waiting for connections...\n\n");

  char read[kMaxMessageSize];
  char address_buffer[kAddressSize];
  while (1) {
    fd_set reads;
    reads = master;
    struct timeval timeout;
    // TODO: This timeout doesn't seem to consistently work? Benchmark
    // and consider using a cross platform poll() instead.
    timeout.tv_usec = 5000;
    //printf("select()\n\n");
    if (select(max_socket + 1, &reads, 0, 0, &timeout) < 0) {
      fprintf(stderr, "select() failed (%d)\n\n",
              network::SocketErrno());
      return;
    }

    bool fd_isset = FD_ISSET(socket_listen, &reads);
    bool queue_has_items = !outgoing_message_queue->Empty();

    // If there is data to read from the socket, read it and cache
    // off the client address.    
    if (fd_isset) {
      struct sockaddr_storage client_address;
      socklen_t client_len = sizeof(client_address);
      memset(&read[0], 0, kMaxMessageSize);
      int bytes_received = recvfrom(
          socket_listen, read, kMaxMessageSize, 0,
          (struct sockaddr*)&client_address, &client_len);
      if (bytes_received < 1) {
        fprintf(stderr, "connection closed. (%d)\n\n",
                network::SocketErrno());
        continue;
      }
      memset(&address_buffer[0], 0, kAddressSize);
      getnameinfo((struct sockaddr*)&client_address, client_len,
                  address_buffer, kAddressSize, 0, 0,
                  NI_NUMERICHOST);
      //printf("Server Message from %s data: %.*s bytes: %i\n\n",
      //       address_buffer, bytes_received, read, bytes_received);
      Message msg;
      msg.data = (char*)malloc(bytes_received);
      memcpy(msg.data, &read[0], bytes_received);
      msg.size = bytes_received;
      incoming_message_queue->Enqueue(msg);
      if (!ClientExists(address_buffer)) {
        // Save off client connections? When do we remove from this list?
        ClientConnection connection;
        connection.client_address = client_address;
        connection.client_len = client_len;
        strncpy(connection.address_buffer, address_buffer,
                kAddressSize);
        kClients[kClientCount++] = connection;
        printf("Caching address %s\n\n", address_buffer);
      }
    }

    // If there is a message in the queue send it to all connected
    // clients. 
    // TODO: I think for managing interested clients we do a simple
    // timeout algorithm. Respond to all clients who have sent us a
    // msg within the last N milliseconds.
    if (queue_has_items) {
      Message msg = outgoing_message_queue->Dequeue();
      do {
        printf(" Message on queue: %.*s\n\n", msg.size, msg.data);
        SendToAllClients(socket_listen, msg);
        msg = outgoing_message_queue->Dequeue();
      } while(msg.size != 0);
    }

    // If the server is supposed to stop, stop it.
    if (outgoing_message_queue->IsStopped()) {
      return;
    }
  }
}

}  // anonymous

std::thread Create(const char* port,
                   MessageQueue* outgoing_message_queue,
                   MessageQueue* incoming_message_queue) {
  return std::thread(StartServer, port, outgoing_message_queue,
                    incoming_message_queue);
}

}  // server

}  // network
