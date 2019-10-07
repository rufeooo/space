#include "gtest/gtest.h"

#include <chrono>
#include <thread>

#include "server.h"
#include "client.h"
#include "message_queue.h"

TEST(Server, ClientHappyPath) {
  using namespace std::chrono_literals;
  network::MessageQueue server_outgoing_message_queue;
  network::MessageQueue server_incoming_message_queue;
  std::thread server_thread
      = network::server::Create("7890",
                                &server_outgoing_message_queue,
                                &server_incoming_message_queue);
  // Give the server a little time to spin  up
  std::cout << "waiting for server to start..."
            << std::endl << std::endl;
  std::this_thread::sleep_for(3s);
  network::MessageQueue client_outgoing_message_queue;
  network::MessageQueue client_incoming_message_queue;
  std::thread client_thread
      = network::client::Create("127.0.0.1", "7890",
                                &client_outgoing_message_queue,
                                &client_incoming_message_queue);
  std::cout << "waiting for client to start..."
            << std::endl << std::endl;
  std::this_thread::sleep_for(3s);
  // In order for clients to start receiving messages from server they
  // need to first send them a message.
  {
    network::Message client_outgoing_msg;
    client_outgoing_msg.data = (char*)malloc(1);
    client_outgoing_msg.data[0] = '0';
    client_outgoing_msg.size = 1;
    client_outgoing_message_queue.Enqueue(client_outgoing_msg);
    
    network::Message server_incoming_msg;
    do {
      server_incoming_msg = server_incoming_message_queue.Dequeue();
    } while  (server_incoming_msg.size == 0);
    ASSERT_EQ(server_incoming_msg.size, 1);
    ASSERT_EQ(strncmp(server_incoming_msg.data, "0", 1), 0);
  }
  // Now validate the client and server can send messages
  // back and forth.
  {
    network::Message server_outgoing_msg;
    server_outgoing_msg.data = (char*)malloc(3);
    server_outgoing_msg.data[0] = '1';
    server_outgoing_msg.data[1] = '2';
    server_outgoing_msg.data[2] = '3';
    server_outgoing_msg.size = 3;
    server_outgoing_message_queue.Enqueue(server_outgoing_msg);

    network::Message client_incoming_msg;
    do {
      client_incoming_msg = client_incoming_message_queue.Dequeue();
    } while (client_incoming_msg.size == 0);
    ASSERT_EQ(client_incoming_msg.size, 3);
    ASSERT_EQ(strncmp(client_incoming_msg.data, "123", 3), 0);
  }
  {
    network::Message server_outgoing_msg;
    server_outgoing_msg.data = (char*)malloc(5);
    server_outgoing_msg.data[0] = 'h';
    server_outgoing_msg.data[1] = 'e';
    server_outgoing_msg.data[2] = 'l';
    server_outgoing_msg.data[3] = 'l';
    server_outgoing_msg.data[4] = 'o';
    server_outgoing_msg.size = 5;
    server_outgoing_message_queue.Enqueue(server_outgoing_msg);

    network::Message client_incoming_msg;
    do {
      client_incoming_msg = client_incoming_message_queue.Dequeue();
    } while (client_incoming_msg.size == 0);
    ASSERT_EQ(client_incoming_msg.size, 5);
    ASSERT_EQ(strncmp(client_incoming_msg.data, "hello", 5), 0);
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
