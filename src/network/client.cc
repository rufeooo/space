#include "client.h"

#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>

#include "platform/platform.cc"

namespace network
{
namespace client
{
static OnMsgReceived _OnMsgReceived;

constexpr int kMaxPacketSize = 1024;

struct ClientState {
  // Thread that the client is running on.
  std::thread client_thread;

  std::atomic<bool> client_running = false;

  const char* hostname;

  const char* port;

  Udp4 client_socket;

  uint8_t receive_buffer[kMaxPacketSize];
};

static ClientState kClientState;

bool
SetupClientSocket()
{
  return udp::GetAddr4(kClientState.hostname, kClientState.port,
                       &kClientState.client_socket);
}

void
RunClientLoop()
{
  uint16_t bytes_received;
  if (!udp::ReceiveFrom(kClientState.client_socket,
                        sizeof(ClientState::receive_buffer),
                        kClientState.receive_buffer, &bytes_received)) {
    if (udp_errno) {
      std::cout << "network failed: " << udp_errno << std::endl;
      kClientState.client_running = false;
    }
    return;
  }

  _OnMsgReceived((uint8_t*)&kClientState.receive_buffer[0], bytes_received);
}

void
RunClient()
{
  while (kClientState.client_running) {
    RunClientLoop();
  }
}

// namespace

void
Setup(OnMsgReceived on_msg_received_callback)
{
  _OnMsgReceived = on_msg_received_callback;
}

// Start client and begin receiving messages from the hostname.
bool
Start(const char* hostname, const char* port)
{
  if (!udp::Init()) {
    std::cout << "Failed to initialize." << std::endl;
    return false;
  }

  kClientState.hostname = hostname;
  kClientState.port = port;

  if (!SetupClientSocket()) {
    return false;
  }

  kClientState.client_thread = std::thread(RunClient);
  kClientState.client_running = true;

  return true;
}

void
Stop()
{
  auto& client_thread = kClientState.client_thread;
  if (!client_thread.joinable()) return;
  kClientState.client_running = false;
  client_thread.join();
}

void
Send(uint8_t* buffer, int size)
{
  if (!kClientState.client_running) return;
  udp::Send(kClientState.client_socket, buffer, size);
}

}  // namespace client

}  // namespace network
