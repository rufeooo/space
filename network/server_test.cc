#include "gtest/gtest.h"

#include <iostream>

#include "server.h"
#include "message_queue.h"

SOCKET CreateSimpleClient(
    const std::string& hostname, const std::string& port,
    const std::string& msg, struct addrinfo** remote_addrinfo) {
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
  *remote_addrinfo = peer_address;
  return socket_peer;
}

TEST(Server, ServerHappyPath) {
  network::MessageQueue message_queue;
  std::thread server_thread
      = network::server::Create("7890", &message_queue);
  // Create a client connection which the server will respond to when
  // new events enter its message queue.
  struct addrinfo* remote_addrinfo;
  SOCKET client_socket
      = CreateSimpleClient("127.0.0.1", "7890", "Connect",
                           &remote_addrinfo);
  ASSERT_TRUE(network::SocketIsValid(client_socket));
  // Enqueue a 5 byte message of 'hello' non-null terminated to be
  // send to the simple client.
  network::Message msg_one;
  msg_one.data = (char*)malloc(5);
  msg_one.data[0] = 'h'; msg_one.data[1] = 'e'; msg_one.data[2] = 'l';
  msg_one.data[3] = 'l'; msg_one.data[4] = 'o'; msg_one.size = 5;
  message_queue.Enqueue(msg_one);


  network::Message msg_two;
  msg_two.data = (char*)malloc(3);
  msg_two.data[0] = '1'; msg_two.data[1] = '2'; msg_two.data[2] = '3';
  msg_two.size = 3;
  message_queue.Enqueue(msg_two);

  // Receive messages.
  char read[1024];
  struct sockaddr_storage client_address;
  socklen_t client_len = sizeof(client_address);

  memset(&read, 0, 1024);
  int bytes_received = recvfrom(
        client_socket, read, 1024, 0,
        (struct sockaddr*)&client_address, &client_len);
  std::cout << "recvfrom() done" << std::endl; 
  ASSERT_EQ(bytes_received, 5);
  ASSERT_EQ(strcmp(read, "hello"), 0);

  memset(&read, 0, 1024);
  bytes_received = recvfrom(
        client_socket, read, 1024, 0,
        (struct sockaddr*)&client_address, &client_len);
  std::cout << "recvfrom() done" << std::endl; 
  ASSERT_EQ(bytes_received, 3);
  ASSERT_EQ(strcmp(read, "123"), 0);

  message_queue.Stop();
  server_thread.join();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
