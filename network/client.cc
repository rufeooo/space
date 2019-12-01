#include "client.h"

#include <atomic>
#include <iostream>
#include <thread>

namespace network {

namespace client {

namespace {

static OnMsgReceived _OnMsgReceived;

constexpr int kMaxPacketSize = 1024;

struct ClientState {
  // Thread that the client is running on.
  std::thread client_thread;

  std::atomic<bool> client_running = false;

  addrinfo* host_address;

  std::string hostname;

  std::string port;

  char address_buffer[100];

  char service_buffer[100];

  SOCKET client_socket;

  fd_set master_fd;

  char read_buffer[kMaxPacketSize];
};

static ClientState kClientState;

bool SetupHostAddressInfo() {
  struct addrinfo hints = {0};
  hints.ai_socktype = SOCK_DGRAM;  // UDP - use SOCK_STREAM for UDP
  auto& host_address = kClientState.host_address;
  if (getaddrinfo(kClientState.hostname.c_str(),
                  kClientState.port.c_str(),
                  &hints, &host_address)) {
    std::cout << "getaddrinfo() failed: "
              << network::SocketErrno() << std::endl;
    return false;
  }

  if (getnameinfo(host_address->ai_addr, host_address->ai_addrlen,
              kClientState.address_buffer,
              sizeof(kClientState.address_buffer),
              kClientState.service_buffer,
              sizeof(kClientState.service_buffer), NI_NUMERICHOST)) {
    std::cout << "getnameinfo() failed: "
              << network::SocketErrno() << std::endl;
    return false;
  }

  return true;
}

bool SetupClientSocket() {
  auto& client_socket = kClientState.client_socket;
  auto& host_address = kClientState.host_address;

  client_socket = socket(
      host_address->ai_family, host_address->ai_socktype,
      host_address->ai_protocol);

  if (!network::SocketIsValid(client_socket)) {
    std::cout << "socket() failed: "
              << network::SocketErrno() << std::endl;
    return false;
  }

  return true;
}


bool RunClientLoop(timeval timeout, SOCKET max_socket) {
  fd_set reads;
  reads = kClientState.master_fd;

  if (select(max_socket + 1, &reads, 0, 0, &timeout) < 0) {
    std::cout << "select() failed: " << network::SocketErrno() << std::endl;
    return false;
  }

  auto& client_socket = kClientState.client_socket;
  if (FD_ISSET(client_socket, &reads)) {
    int bytes_received = recvfrom(
        client_socket, kClientState.read_buffer, kMaxPacketSize,
        0, nullptr, nullptr);

    assert(bytes_received < kMaxPacketSize);
    if (bytes_received < 1) {
      std::cout << "connection closed." << std::endl;
      return false;
    }

    _OnMsgReceived((uint8_t*)&kClientState.read_buffer[0], bytes_received);
  }

  return true;
}

void RunClient() {
  FD_ZERO(&kClientState.master_fd);
  FD_SET(kClientState.client_socket, &kClientState.master_fd);
  SOCKET max_socket = kClientState.client_socket;

  struct timeval timeout = {};
  timeout.tv_sec = 0; 
  timeout.tv_usec = 5000;

  while (kClientState.client_running) {
    if (!RunClientLoop(timeout, max_socket)) {
      break;
    }
  }
}

}  // anonymous

void Setup(OnMsgReceived on_msg_received_callback) {
  _OnMsgReceived = on_msg_received_callback;
}

// Start client and begin receiving messages from the hostname.
bool Start(const char* hostname, const char* port) {
  if (!SocketInit()) {
    std::cout << "Failed to initialize." << std::endl;
    return false;
  }

  kClientState.hostname = hostname;
  kClientState.port = port;

  if (!SetupHostAddressInfo()) {
    return false;
  }

  if (!SetupClientSocket()) {
    return false;
  }

  kClientState.client_thread = std::thread(RunClient);
  kClientState.client_running = true;

  return true;
}

void Stop() {
  auto& client_thread = kClientState.client_thread;
  if (!client_thread.joinable()) return;
  kClientState.client_running = false;
  client_thread.join();
}

void Send(uint8_t* buffer, int size) {
  sendto(kClientState.client_socket,
         (const char*)buffer, size, 0,
         kClientState.host_address->ai_addr,
         kClientState.host_address->ai_addrlen);
}

}  // client

}  // network
