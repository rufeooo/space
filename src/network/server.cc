#include "server.h"

#include <atomic>
#include <cassert>
#include <cstring>
#include <iostream>
#include <thread>

#include "platform/platform.cc"

namespace network
{
namespace server
{
static OnClientConnected _OnClientConnected;
static OnMsgReceived _OnMsgReceived;

// ...
constexpr int kMaxPacketSize = 1024;

struct ServerState {
  // Thread that the server is running on.
  std::thread server_thread;

  // Set to true if the server is running. Switch to false
  // to kill the server.
  std::atomic<bool> server_running = false;

  // ...
  Udp4 clients[kMaxClients];

  // ...
  int client_count;

  // ...
  Udp4 server_socket;

  // ...
  const char* server_address;

  // ...
  const char* port;

  // ...
  uint8_t receive_buffer[kMaxPacketSize];
};

static ServerState kServerState;

int
GetClientId(Udp4* peer)
{
  for (int i = 0; i < kMaxClients; ++i) {
    if (memcmp(kServerState.clients[i].socket_address, peer->socket_address,
               sizeof(Udp4::socket_address)) == 0) {
      return i;
    }
  }
  return -1;
}

bool
SetupListenSocket()
{
  if (!udp::GetAddr4(kServerState.server_address, kServerState.port,
                     &kServerState.server_socket)) {
    return false;
  }

  if (!udp::Bind(kServerState.server_socket)) {
    std::cout << "bind failed: " << udp_errno << std::endl;
    return false;
  }

  return true;
}

void
RunServerLoop()
{
  uint16_t bytes_received;
  Udp4 remote;
  if (!udp::ReceiveAny(kServerState.server_socket,
                       sizeof(ServerState::receive_buffer),
                       kServerState.receive_buffer, &bytes_received, &remote)) {
    if (udp_errno) {
      fprintf(stderr, "connection closed. (%d)\n\n", udp_errno);
      kServerState.server_running = false;
    }
    return;
  }

  int id = GetClientId(&remote);

  // TODO: handshake

  if (id == -1) {
    // Save off client connections? When do we remove from this list?
    id = kServerState.client_count;
    kServerState.clients[id] = remote;
    ++kServerState.client_count;
    std::cout << "Client connected: " << id << std::endl;
    _OnClientConnected(id);
  }

  _OnMsgReceived(id, (uint8_t*)&kServerState.receive_buffer[0], bytes_received);
}

void
RunServer()
{
  while (kServerState.server_running) {
    // Run the server loop until it returns false.
    RunServerLoop();
  }
}
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
  if (!udp::Init()) {
    std::cout << "Failed to initialize." << std::endl;
    return false;
  }

  kServerState.server_address = "0.0.0.0";
  kServerState.port = port;

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
Send(int client_id, uint8_t* buffer, uint16_t size)
{
  if (!kServerState.server_running) return;
  udp::SendTo(kServerState.server_socket, kServerState.clients[client_id],
              buffer, size);
}

}  // namespace server
}  // namespace network

