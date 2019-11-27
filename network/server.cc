#include "server.h"

#include <iostream>
#include <atomic>
#include <thread>

#include "network.h"

namespace network {

namespace server {

namespace {

static OnClientConnected _OnClientConnected;
static OnMsgRecieved _OnMsgReceived;

// ...
constexpr int kAddressSize = 256;

// ...
constexpr int kMaxClients = 10;

// ...
constexpr int kMaxPacketSize = 1024;

struct Connection {
  sockaddr_storage client_address = {};

  socklen_t client_len = 0;

  char address_buffer[kAddressSize] = {};
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
  SOCKET listen_socket;

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

int GetClientId(char* address) {
  for (int i = 0; i < kMaxClients; ++i) {
    if (strcmp(kServerState.clients[i].address_buffer, address) == 0) {
      return i;
    }
  }
  return -1;
}

void SetupBindAddressInfo(const char* port) {
  addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;  // UDP - use SOCK_STREAM for UDP
  hints.ai_flags = AI_PASSIVE;
  auto& bind_address = kServerState.bind_address;
  getaddrinfo(0, port, &hints, &bind_address);
}

bool SetupListenSocket() {
  auto& listen_socket = kServerState.listen_socket;
  auto& bind_address = kServerState.bind_address;
  listen_socket =
      socket(bind_address->ai_family, bind_address->ai_socktype,
             bind_address->ai_protocol);

  if (!SocketIsValid(listen_socket)) {
    std::cout << "socket() failed: " << network::SocketErrno() << std::endl;
    return false;
  }

  if (bind(listen_socket, bind_address->ai_addr,
           bind_address->ai_addrlen)) {
    std::cout << "bind() failed: " << network::SocketErrno() << std::endl;
    return false;
  }

  return true;
}

bool RunServerLoop(timeval timeout, SOCKET max_socket) {
  fd_set reads;
  reads = kServerState.master_fd;

  // TODO: This timeout doesn't seem to consistently work? Benchmark
  // and consider using a cross platform poll() instead.
  if (select(max_socket + 1, &reads, 0, 0, &timeout) < 0) {
    std::cout << "select() failed: " << network::SocketErrno() << std::endl;
    return false;
  }

  // If there is data to read from the socket, read it and cache
  // off the client address.    
  auto& socket_listen = kServerState.listen_socket; 
  if (FD_ISSET(socket_listen, &reads)) {
    sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    int bytes_received = recvfrom(
        socket_listen, kServerState.read_buffer, kMaxPacketSize,
        0, (sockaddr*)&client_address, &client_len);
    assert(bytes_received < kMaxPacketSize);

    if (bytes_received < 1) {
      fprintf(stderr, "connection closed. (%d)\n\n",
              network::SocketErrno());
      return true;
    }

    static char address_buffer[kAddressSize];
    memset(&address_buffer[0], 0, kAddressSize);
    getnameinfo((sockaddr*)&client_address, client_len,
                address_buffer, kAddressSize, 0, 0, NI_NUMERICHOST);

    int id = GetClientId(address_buffer);

    if (id == -1) {
      // Save off client connections? When do we remove from this list?
      Connection connection;
      connection.client_address = client_address;
      connection.client_len = client_len;
      strncpy(connection.address_buffer, address_buffer, kAddressSize);
      id = kServerState.client_count;
      kServerState.clients[kServerState.client_count++] = connection;
      std::cout << "Client connected: " << id << std::endl;
      _OnClientConnected(id);
    }

    _OnMsgReceived(id, (uint8_t*)&kServerState.read_buffer[0], bytes_received);
  }

  return true;
}

void RunServer() {
  FD_ZERO(&kServerState.master_fd);
  FD_SET(kServerState.listen_socket, &kServerState.master_fd);
  SOCKET max_socket = kServerState.listen_socket;

  // Timeout that is used in select() called.
  timeval timeout = {};
  timeout.tv_sec = 0;
  timeout.tv_usec = 5000;

  while (kServerState.server_running) {
    // Run the server loop until it returns false.
    if (!RunServerLoop(timeout, max_socket)) {
      break;
    }
  }
}

}  // anonymous

void Setup(
    OnClientConnected on_client_connected_callback,
    OnMsgRecieved on_msg_received_callback) {
  _OnClientConnected = on_client_connected_callback;
  _OnMsgReceived = on_msg_received_callback;
}

bool Start(const char* port) {
  if (!SocketInit()) {
    std::cout << "Failed to initialize." << std::endl;
    return false;
  }

  SetupBindAddressInfo(port);

  if (!SetupListenSocket()) {
    std::cout << "Failed setting up listen socket." << std::endl;
    return false;
  }

  kServerState.port = port;
  kServerState.server_thread = std::thread(RunServer);
  kServerState.server_running = true;

  return true;
}

void Stop() {
  auto& server_thread = kServerState.server_thread;
  if (!server_thread.joinable()) return;
  kServerState.server_running = false;
  server_thread.join();
}

void Send(int client_id, uint8_t* buffer, int size) {
  sendto(kServerState.listen_socket, (char*)buffer, size, 0,
         (sockaddr*)&kServerState.clients[client_id].client_address,
         kServerState.clients[client_id].client_len);
}

}  // server

}  // network
