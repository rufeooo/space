#include "gtest/gtest.h"

#include <chrono>
#include <thread>

#include <flatbuffers/flatbuffers.h>

#include "server.h"
#include "client.h"
#include "message_queue.h"
#include "testdata/packet_generated.h"

TEST(Server, ClientHappyPath) {
  using namespace std::chrono_literals;
  network::OutgoingMessageQueue server_outgoing_message_queue;
  network::IncomingMessageQueue server_incoming_message_queue;
  std::thread server_thread
      = network::server::Create("7890",
                                &server_outgoing_message_queue,
                                &server_incoming_message_queue);
  // Give the server a little time to spin  up
  std::cout << "waiting for server to start..."
            << std::endl << std::endl;
  std::this_thread::sleep_for(1s);
  network::OutgoingMessageQueue client_outgoing_message_queue;
  network::IncomingMessageQueue client_incoming_message_queue;
  std::thread client_thread
      = network::client::Create("127.0.0.1", "7890",
                                &client_outgoing_message_queue,
                                &client_incoming_message_queue);
  std::cout << "waiting for client to start..."
            << std::endl << std::endl;
  std::this_thread::sleep_for(1s);
  // In order for clients to start receiving messages from server they
  // need to first send them a message.
  {
    flatbuffers::FlatBufferBuilder fbb;
    auto data = fbb.CreateString("Hello!");
    auto packet = testdata::CreatePacket(fbb, data);
    fbb.Finish(packet);
    client_outgoing_message_queue.Enqueue(fbb.Release());
    network::Message server_incoming_msg;
    do {
      server_incoming_msg = server_incoming_message_queue.Dequeue();
    } while  (server_incoming_msg.size == 0);
    auto* received_packet = testdata::GetPacket(
        (const void*)server_incoming_msg.data);
    ASSERT_EQ(std::string(received_packet->data()->c_str()),
              std::string("Hello!"));
    free(server_incoming_msg.data);
  }
  {
    flatbuffers::FlatBufferBuilder fbb;
    auto data = fbb.CreateString("Hello Back!");
    auto packet = testdata::CreatePacket(fbb, data);
    fbb.Finish(packet);
    server_outgoing_message_queue.Enqueue(fbb.Release());
    network::Message client_incoming_msg;
    do {
      client_incoming_msg = client_incoming_message_queue.Dequeue();
    } while  (client_incoming_msg.size == 0);
    auto* received_packet = testdata::GetPacket(
        (const void*)client_incoming_msg.data);
    ASSERT_EQ(std::string(received_packet->data()->c_str()),
              std::string("Hello Back!"));
    free(client_incoming_msg.data);
  }
  server_outgoing_message_queue.Stop();
  client_outgoing_message_queue.Stop();

  server_thread.join();
  client_thread.join();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
