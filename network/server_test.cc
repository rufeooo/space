#include "gtest/gtest.h"

#include <iostream>

#include <flatbuffers/flatbuffers.h>

#include "server.h"
#include "message_queue.h"
#include "testdata/packet_generated.h"

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

char* ReadSocketData(SOCKET client_socket) {
  char read[network::kMaxMessageSize];
  memset(&read, 0, network::kMaxMessageSize);
  int bytes_received = recvfrom(
      client_socket, read, network::kMaxMessageSize, 0, nullptr,
      nullptr);
  std::cout << "Received " << bytes_received << " bytes of data."
            << std::endl;
  char* data = (char*)malloc(bytes_received);
  memcpy(data, &read[0], bytes_received);
  return data;
}

TEST(Server, ServerHappyPath) {
  network::OutgoingMessageQueue outgoing_message_queue;            
  network::IncomingMessageQueue incoming_message_queue;
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
  ASSERT_EQ(strncmp((char*)msg.data, "Connect", msg.size), 0);
  {
    // Enqueue a flatbuffer with message 'hello'
    flatbuffers::FlatBufferBuilder fbb;
    auto data = fbb.CreateString("Hello!");
    auto packet = testdata::CreatePacket(fbb, data);
    fbb.Finish(packet);
    outgoing_message_queue.Enqueue(fbb.Release());
    char* socket_data = ReadSocketData(client_socket);
    auto* received_packet = testdata::GetPacket(
        (const void*)socket_data);
    std::cout << received_packet->data()->c_str() << std::endl;
    ASSERT_EQ(std::string(received_packet->data()->c_str()),
              std::string("Hello!"));
    free(socket_data);
  }

  {
    // Enqueue a flatbuffer with message 'hello'
    flatbuffers::FlatBufferBuilder fbb;
    auto data = fbb.CreateString(
        "The quick brown fox jumps over the lazy dog.");
    auto packet = testdata::CreatePacket(fbb, data);
    fbb.Finish(packet);
    outgoing_message_queue.Enqueue(fbb.Release());
    char* socket_data = ReadSocketData(client_socket);
    auto* received_packet = testdata::GetPacket(
        (const void*)socket_data);
    std::cout << received_packet->data()->c_str() << std::endl;
    ASSERT_EQ(std::string(received_packet->data()->c_str()),
              std::string("The quick brown fox jumps over the lazy "
                          "dog."));
    free(socket_data);
  }
  incoming_message_queue.Stop();
  server_thread.join();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
