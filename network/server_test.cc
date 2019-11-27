#include "gtest/gtest.h"

#include <iostream>
#include <thread>

#include "server.h"

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

static int kStuffCalled = 0;

void OnClientConnected(int client_id) {
  ASSERT_EQ(client_id, 0);
  ++kStuffCalled;
}

void OnMsgRecieved(int client_id, uint8_t* data, int size) {
  ASSERT_EQ(size, 7);
  ASSERT_EQ(strncmp((char*)data, "Connect", size), 0);
  ++kStuffCalled;
}

TEST(Server, ServerHappyPath) {
  using namespace std::chrono_literals;
  network::server::Setup(&OnClientConnected, &OnMsgRecieved);
  ASSERT_TRUE(network::server::Start("7890"));
  // Create a client connection which the server will respond to when
  // new events enter its message queue.
  SOCKET client_socket
      = CreateSimpleClient("127.0.0.1", "7890", "Connect");
  ASSERT_TRUE(network::SocketIsValid(client_socket));
  std::this_thread::sleep_for(2s);
  ASSERT_EQ(kStuffCalled, 2);
  network::server::Stop();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
