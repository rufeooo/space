#include "server.h"

#include <atomic>
#include <cassert>
#include <cstring>
#include <iostream>
#include <thread>

#include "network.cc"

namespace network
{
namespace server
{
static OnClientConnected _OnClientConnected;
static OnMsgReceived _OnMsgReceived;

// ...
constexpr int kMaxPacketSize = 1024;

struct Connection {
  sockaddr_storage client_address = {};

  socklen_t client_len = 0;
};

struct ServerState {
  // Thread that the server is running on.
  std::thread server_thread;

  // Set to true if the server is running. Switch to false
  // to kill the server.
  std::atomic<bool> server_running = false;

  // ...
  Connection clients[kMaxClients];

  // ...
  int client_count;

  // ...
  SOCKET server_socket;

  // ...
  addrinfo* bind_address;

  // ...
  std::string port;

  // ...
  fd_set master_fd;

  // ...
  char read_buffer[kMaxPacketSize];
};

static ServerState kServerState;

int
GetClientId(const sockaddr_storage& client_address)
{
  for (int i = 0; i < kMaxClients; ++i) {
    if (memcmp(&kServerState.clients[i].client_address, &client_address,
               sizeof(client_address)) == 0) {
      return i;
    }
  }
  return -1;
}

void
SetupBindAddressInfo()
{
  addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;  // UDP - use SOCK_STREAM for UDP
  hints.ai_flags = AI_PASSIVE;
  auto& bind_address = kServerState.bind_address;
  getaddrinfo(0, kServerState.port.c_str(), &hints, &bind_address);
}

bool
SetupListenSocket()
{
  auto& server_socket = kServerState.server_socket;
  auto& bind_address = kServerState.bind_address;
  server_socket = socket(bind_address->ai_family, bind_address->ai_socktype,
                         bind_address->ai_protocol);

  if (!SocketIsValid(server_socket)) {
    std::cout << "socket() failed: " << network::SocketErrno() << std::endl;
    return false;
  }

  if (bind(server_socket, bind_address->ai_addr, bind_address->ai_addrlen)) {
    std::cout << "bind() failed: " << network::SocketErrno() << std::endl;
    return false;
  }

  return true;
}

void
RunServerLoop(timeval timeout, SOCKET max_socket)
{
  fd_set reads;
  reads = kServerState.master_fd;

  if (select(max_socket + 1, &reads, 0, 0, &timeout) < 0) {
    std::cout << "select() failed: " << network::SocketErrno() << std::endl;
    kServerState.server_running = false;
    return;
  }

  // If there is data to read from the socket, read it and cache
  // off the client address.
  auto& socket_listen = kServerState.server_socket;
  if (FD_ISSET(socket_listen, &reads)) {
    sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    int bytes_received =
        recvfrom(socket_listen, kServerState.read_buffer, kMaxPacketSize, 0,
                 (sockaddr*)&client_address, &client_len);
    assert(bytes_received < kMaxPacketSize);

    if (bytes_received < 1) {
      fprintf(stderr, "connection closed. (%d)\n\n", network::SocketErrno());
      kServerState.server_running = false;
      return;
    }

    int id = GetClientId(client_address);

    if (id == -1) {
      // Save off client connections? When do we remove from this list?
      Connection connection;
      connection.client_address = client_address;
      connection.client_len = client_len;
      id = kServerState.client_count;
      kServerState.clients[kServerState.client_count++] = connection;
      std::cout << "Client connected: " << id << std::endl;
      _OnClientConnected(id);
    }

    _OnMsgReceived(id, (uint8_t*)&kServerState.read_buffer[0], bytes_received);
  }
}

void
RunServer()
{
  FD_ZERO(&kServerState.master_fd);
  FD_SET(kServerState.server_socket, &kServerState.master_fd);
  SOCKET max_socket = kServerState.server_socket;

  // Timeout that is used in select() called.
  timeval timeout = {};
  timeout.tv_sec = 0;
  timeout.tv_usec = 5000;

  while (kServerState.server_running) {
    // Run the server loop until it returns false.
    RunServerLoop(timeout, max_socket);
  }
}

// namespace

void
Setup(OnClientConnected on_client_connected_callback,
      OnMsgReceived on_msg_received_callback)
{
  _OnClientConnected = on_client_connected_callback;
  _OnMsgReceived = on_msg_received_callback;
}

bool
Start(const char* port)
{
  if (!SocketInit()) {
    std::cout << "Failed to initialize." << std::endl;
    return false;
  }

  kServerState.port = port;

  SetupBindAddressInfo();

  if (!SetupListenSocket()) {
    std::cout << "Failed setting up listen socket." << std::endl;
    return false;
  }

  kServerState.server_thread = std::thread(RunServer);
  kServerState.server_running = true;

  return true;
}

void
Stop()
{
  auto& server_thread = kServerState.server_thread;
  if (!server_thread.joinable()) return;
  kServerState.server_running = false;
  server_thread.join();
}

void
Send(int client_id, uint8_t* buffer, int size)
{
  if (!kServerState.server_running) return;
  sendto(kServerState.server_socket, (char*)buffer, size, 0,
         (sockaddr*)&kServerState.clients[client_id].client_address,
         kServerState.clients[client_id].client_len);
}

}  // namespace server

}  // namespace network
