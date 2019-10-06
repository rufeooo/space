#include "gtest/gtest.h"

#include <iostream>

#include "server.h"
#include "message_queue.h"

SOCKET CreateSimpleClient(
    const std::string& hostname, const std::string& port,
    const std::string& msg) {
  if (!network::SocketInit()) {
    printf("Failed to initialize...\n\n");
    return INVALID_SOCKET;
  }
  struct addrinfo hints = {0};
  hints.ai_socktype = SOCK_DGRAM;  // UDP - use SOCK_STREAM for UDP
  struct addrinfo* peer_address;
  if (getaddrinfo(hostname.c_str(), port.c_str(), &hints,
                  &peer_address)) {
    fprintf(stderr, "getaddrinfo() failed (%d)\n\n",
            network::SocketErrno());
    return INVALID_SOCKET;
  }

  char address_buffer[100];
  char service_buffer[100];
  getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
              address_buffer, sizeof(address_buffer),
              service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
  SOCKET socket_peer;
  socket_peer = socket(
      peer_address->ai_family, peer_address->ai_socktype,
      peer_address->ai_protocol);
  if (!network::SocketIsValid(socket_peer)) {
    fprintf(stderr, "socket() failed (%d)\n\n",
            network::SocketErrno());
    return INVALID_SOCKET;
  }
  int bytes_sent = sendto(socket_peer, msg.c_str(), msg.size(), 0,
                          peer_address->ai_addr,
                          peer_address->ai_addrlen);
  return socket_peer;
}

TEST(Server, ServerHappyPath) {
  network::MessageQueue outgoing_message_queue;
  network::MessageQueue incoming_message_queue;
  std::thread server_thread
      = network::server::Create("7890", &outgoing_message_queue,
                                &incoming_message_queue);
  // Create a client connection which the server will respond to when
  // new events enter its message queue.
  SOCKET client_socket
      = CreateSimpleClient("127.0.0.1", "7890", "Connect");
  ASSERT_TRUE(network::SocketIsValid(client_socket));
  // The server should now contain a "Connect" message from the
  // connecting client.
  network::Message msg;
  // Constantly dequeue the incoming message queue until the message
  // is received.
  do {
    msg = incoming_message_queue.Dequeue();
  } while (msg.size == 0);
  ASSERT_EQ(msg.size, 7);
  ASSERT_EQ(strncmp(msg.data, "Connect", msg.size), 0);
  // Enqueue a 5 byte message of 'hello' non-null terminated to be
  // send to the simple client.
  network::Message msg_one;
  msg_one.data = (char*)malloc(5);
  msg_one.data[0] = 'h'; msg_one.data[1] = 'e'; msg_one.data[2] = 'l';
  msg_one.data[3] = 'l'; msg_one.data[4] = 'o'; msg_one.size = 5;
  outgoing_message_queue.Enqueue(msg_one);


  network::Message msg_two;
  msg_two.data = (char*)malloc(3);
  msg_two.data[0] = '1'; msg_two.data[1] = '2'; msg_two.data[2] = '3';
  msg_two.size = 3;
  outgoing_message_queue.Enqueue(msg_two);

  // Receive messages.
  char read[network::kMaxMessageSize];
  struct sockaddr_storage client_address;
  socklen_t client_len = sizeof(client_address);

  memset(&read, 0, network::kMaxMessageSize);
  int bytes_received = recvfrom(
        client_socket, read, network::kMaxMessageSize, 0,
        (struct sockaddr*)&client_address, &client_len);
  ASSERT_EQ(bytes_received, 5);
  ASSERT_EQ(strncmp(read, "hello", 5), 0);

  memset(&read, 0, network::kMaxMessageSize);
  bytes_received = recvfrom(
        client_socket, read, network::kMaxMessageSize, 0,
        (struct sockaddr*)&client_address, &client_len);
  ASSERT_EQ(bytes_received, 3);
  ASSERT_EQ(strncmp(read, "123", 3), 0);

  outgoing_message_queue.Stop();
  server_thread.join();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
