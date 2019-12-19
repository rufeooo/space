#include "gtest/gtest.h"

#include <iostream>
#include <thread>

#include "server.h"

struct Client {
  SOCKET socket;
  addrinfo* address;
};

Client
CreateSimpleClient(const std::string& hostname, const std::string& port)
{
  Client client;
  if (!network::SocketInit()) {
    printf("Failed to initialize...\n\n");
    return client;
  }

  struct addrinfo hints = {0};
  hints.ai_socktype = SOCK_DGRAM;  // UDP - use SOCK_STREAM for UDP
  if (getaddrinfo(hostname.c_str(), port.c_str(), &hints, &client.address)) {
    fprintf(stderr, "getaddrinfo() failed (%d)\n\n", network::SocketErrno());
    return client;
  }

  char address_buffer[100];
  char service_buffer[100];
  getnameinfo(client.address->ai_addr, client.address->ai_addrlen,
              address_buffer, sizeof(address_buffer), service_buffer,
              sizeof(service_buffer), NI_NUMERICHOST);

  client.socket = socket(client.address->ai_family, client.address->ai_socktype,
                         client.address->ai_protocol);

  if (!network::SocketIsValid(client.socket)) {
    fprintf(stderr, "socket() failed (%d)\n\n", network::SocketErrno());
    return client;
  }

  return client;
}

int
SendData(Client& client, const std::string& msg)
{
  return sendto(client.socket, msg.c_str(), msg.size(), 0,
                client.address->ai_addr, client.address->ai_addrlen);
}

static int kStuffCalled = 0;

void
OnClientConnected(int client_id)
{
  ASSERT_EQ(client_id, 0);
  ++kStuffCalled;
}

void
OnMsgReceived(int client_id, uint8_t* data, int size)
{
  ASSERT_EQ(size, 5);
  ASSERT_EQ(strncmp((char*)data, "hello", size), 0);
  ++kStuffCalled;
}

TEST(Server, ServerHappyPath)
{
  using namespace std::chrono_literals;
  network::server::Setup(&OnClientConnected, &OnMsgReceived);
  ASSERT_TRUE(network::server::Start("7890"));
  // Create a client connection which the server will respond to when
  // new events enter its message queue.
  Client client = CreateSimpleClient("127.0.0.1", "7890");
  SendData(client, "hello");
  ASSERT_TRUE(network::SocketIsValid(client.socket));
  std::this_thread::sleep_for(2s);
  ASSERT_EQ(kStuffCalled, 2);
  network::server::Stop();
}

int
main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
