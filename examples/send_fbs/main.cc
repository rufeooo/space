#include <chrono>
#include <thread>
#include <iostream>

#include "packet_generated.h"
#include "flatbuffers/flatbuffers.h"

#include "network/client.h"
#include "network/message_queue.h"
#include "network/server.h"

int main(int argc, char** argv) {
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
  std::this_thread::sleep_for(2s);
  network::MessageQueue client_outgoing_message_queue;
  network::MessageQueue client_incoming_message_queue;
  std::thread client_thread
      = network::client::Create("127.0.0.1", "7890",
                                &client_outgoing_message_queue,
                                &client_incoming_message_queue);
  std::cout << "waiting for client to start..."
            << std::endl << std::endl;
  std::this_thread::sleep_for(2s);

  flatbuffers::FlatBufferBuilder fbb;
  auto data = fbb.CreateString("Hello!");
  auto packet = send_fbs::CreatePacket(fbb, data);
  fbb.Finish(packet);

  network::Message client_msg;
  client_msg.data = (char*)fbb.GetBufferPointer();
  client_msg.size = fbb.GetSize();
  client_outgoing_message_queue.Enqueue(client_msg);

  std::this_thread::sleep_for(2s);

  network::Message server_msg
      = server_incoming_message_queue.Dequeue();

  auto* received_packet
      = send_fbs::GetPacket((const void*)server_msg.data);
  std::cout << received_packet->data()->c_str() << std::endl;
  free(server_msg.data);
  //std::cout << server_msg.size << std::endl;
  //auto packet = send_fbs 

  server_outgoing_message_queue.Stop();
  client_outgoing_message_queue.Stop();

  server_thread.join();
  client_thread.join();

  return 0;
}
