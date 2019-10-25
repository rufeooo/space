#include "server.h"

#include <thread>

#include <flatbuffers/flatbuffers.h>

#include "network.h"

namespace network {

namespace server {

namespace {

constexpr int kAddressSize = 256;

struct ClientConnection {
  struct sockaddr_storage client_address = {};
  socklen_t client_len = 0;
  char address_buffer[kAddressSize] = {};
};

// Lets keep it simple for now and just allow 10 clients. To receive
// messages back from us.
static struct ClientConnection kClients[kMaxClients]; 
static int kClientCount = 0;

int GetClientId(char* address) {
  for (int i = 0; i < kClientCount; ++i) {
    if (strcmp(kClients[i].address_buffer, address) == 0) {
      return i;
    }
  }
  return -1;
}

void StartServer(const char* port,
                 OutgoingMessageQueue* outgoing_message_queue,
                 IncomingMessageQueue* incoming_message_queue) {
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
  struct timeval timeout = {};
  timeout.tv_sec = 0;
  timeout.tv_usec = 5000;
  while (1) {
    fd_set reads;
    reads = master;
    // TODO: This timeout doesn't seem to consistently work? Benchmark
    // and consider using a cross platform poll() instead.
    if (select(max_socket + 1, &reads, 0, 0, &timeout) < 0) {
      fprintf(stderr, "select() failed (%d)\n\n",
              network::SocketErrno());
      return;
    }

    //printf("select()\n\n");

    //bool queue_has_items = !outgoing_message_queue->Empty();
    // If there is data to read from the socket, read it and cache
    // off the client address.    
    if (FD_ISSET(socket_listen, &reads)) {
      printf("fd_isset\n\n");
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
      int id = GetClientId(address_buffer);
      if (id == -1) {
        // Save off client connections? When do we remove from this
        // list?
        ClientConnection connection;
        connection.client_address = client_address;
        connection.client_len = client_len;
        strncpy(connection.address_buffer, address_buffer,
                kAddressSize);
        id = kClientCount;
        outgoing_message_queue->AddRecipient(id);
        kClients[kClientCount++] = connection;
        printf("Caching address %s client_id(%d)\n\n",
               address_buffer, id);
      }
      Message msg;
      msg.data = (char*)malloc(bytes_received);
      memcpy(msg.data, &read[0], bytes_received);
      msg.size = bytes_received;
      msg.client_id = id;
      printf("gots bytes: %d\n\n", bytes_received);
      incoming_message_queue->Enqueue(msg); 
    }

    // If there is a message in the queue send it to all connected
    // clients. 
    // TODO: I think for managing interested clients we do a simple
    // timeout algorithm. Respond to all clients who have sent us a
    // msg within the last N milliseconds.
    // TODO: Is this the best way to manage sending messages to certain
    // clients?
    flatbuffers::DetachedBuffer msg
          = outgoing_message_queue->Dequeue();
    while (msg.size() != 0) {
      for (int client_id : outgoing_message_queue->Recipients()) {
        sendto(socket_listen, (char*)msg.data(), msg.size(), 0,
               (struct sockaddr*)&kClients[client_id].client_address,
               kClients[client_id].client_len);
        msg = outgoing_message_queue->Dequeue();
      }
    }

    // If the server is supposed to stop, stop it.
    // TODO: Probably do something better here for stopping server.
    if (incoming_message_queue->IsStopped()) {
      return;
    }
  }
}

}  // anonymous

std::thread Create(const char* port,
                   OutgoingMessageQueue* outgoing_message_queue,
                   IncomingMessageQueue* incoming_message_queue) {
  return std::thread(StartServer, port, outgoing_message_queue,
                    incoming_message_queue);
}

}  // server

}  // network
