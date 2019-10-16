#include "server.h"

#include <thread>

#include "network.h"

namespace network {

namespace client {

namespace {

void StartClient(const char* hostname, const char* port,
                 OutgoingMessageQueue* outgoing_message_queue,
                 IncomingMessageQueue* incoming_message_queue) {
  if (!SocketInit()) {
    printf("Failed to initialize...\n\n");
    return;
  }

  printf("Configuring remote address...\n\n");
  struct addrinfo hints = {0};
  hints.ai_socktype = SOCK_DGRAM;  // UDP - use SOCK_STREAM for UDP
  struct addrinfo* host_address;
  if (getaddrinfo(hostname, port, &hints, &host_address)) {
    fprintf(stderr, "getaddrinfo() failed (%d)\n\n",
            network::SocketErrno());
    return;
  }

  char address_buffer[100];
  char service_buffer[100];

  getnameinfo(host_address->ai_addr, host_address->ai_addrlen,
              address_buffer, sizeof(address_buffer),
              service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
  printf("Name info: %s %s...\n\n", address_buffer, service_buffer);
  printf("Creating client socket...\n\n");

  SOCKET socket_host;
  socket_host = socket(
      host_address->ai_family, host_address->ai_socktype,
      host_address->ai_protocol);

  if (!network::SocketIsValid(socket_host)) {
    fprintf(stderr, "socket() failed (%d)\n\n",
            network::SocketErrno());
    return;
  }


  char read[kMaxMessageSize];
  while (1) {
    fd_set reads;
    FD_ZERO(&reads);
    FD_SET(socket_host, &reads);
    struct timeval timeout;
    // TODO: This timeout doesn't seem to consistently work? Benchmark
    // and consider using a cross platform poll() instead.
    timeout.tv_sec = 0; 
    timeout.tv_usec = 5000;
    //printf("select()\n\n");
    if (select(socket_host + 1, &reads, 0, 0, &timeout) < 0) {
      fprintf(stderr, "select() failed (%d)\n\n",
              network::SocketErrno());
      return;
    }

    //printf("select()\n\n");

    bool fd_isset = FD_ISSET(socket_host, &reads);
    bool queue_has_items = !outgoing_message_queue->Empty();

    if (fd_isset) {
      // Enqueue incoming messages from server.
      memset(&read[0], 0, kMaxMessageSize);
      int bytes_received = recvfrom(
          socket_host, read, kMaxMessageSize, 0, nullptr, nullptr);
      if (bytes_received < 1) {
        fprintf(stderr, "connection closed. (%d)\n\n",
                network::SocketErrno());
        continue;
      }
      printf("Message from host data: %.*s bytes: %i\n\n",
             bytes_received, read, bytes_received);
      Message msg;
      msg.data = (char*)malloc(bytes_received);
      memcpy(msg.data, &read[0], bytes_received);
      msg.size = bytes_received;
      incoming_message_queue->Enqueue(msg);
    }

    if (queue_has_items) {
      flatbuffers::DetachedBuffer msg
          = outgoing_message_queue->Dequeue();
      do {
        // Send outgoing messages to server.
        sendto(socket_host, msg.data(), msg.size(), 0,
               host_address->ai_addr, host_address->ai_addrlen);
        //free(msg.data);
        msg = outgoing_message_queue->Dequeue();
      } while(msg.size() != 0);
    }

    // If the client is supposed to stop, stop it.
    if (outgoing_message_queue->IsStopped()) {
      return;
    }
  }
}


}  // anonymous

std::thread Create(const char* hostname, const char* port,
                   OutgoingMessageQueue* outgoing_message_queue,
                   IncomingMessageQueue* incoming_message_queue) {
  return std::thread(StartClient, hostname, port,
                     outgoing_message_queue, incoming_message_queue);
}

}  // client

}  // network
