#include <iostream>
#include <thread>

#include <gflags/gflags.h>

#include "network/client.h"
#include "network/message_queue.h"
#include "protocol/asteroids_packet_generated.h"

DEFINE_string(hostname, "127.0.0.1",
              "If provided will connect to a game server. Will play "
              "the game singleplayer otherwise.");

DEFINE_string(port, "9843", "Port for this application.");

void SendGarbage(
    network::OutgoingMessageQueue* outgoing_message_queue) {
  flatbuffers::FlatBufferBuilder fbb;
  auto position = asteroids::Vec3(0.f, 0.f, 0.f);
  auto orientation = asteroids::Vec4(0.f, 0.f, 0.f, 0.f);
  auto transform = asteroids::Transform(position, orientation);
  auto acceleration = asteroids::Vec3(0.f, 0.f, 0.f);
  auto velocity = asteroids::Vec3(0.f, 0.f, 0.f);
  auto physics = asteroids::Physics(acceleration, velocity);
  auto player_state = asteroids::PlayerState(0, transform, physics);
  auto packet = asteroids::CreatePacket(fbb, &player_state, nullptr);
  fbb.Finish(packet);
  outgoing_message_queue->Enqueue(fbb.Release());
}


int main(int argc, char** argv) {
  network::OutgoingMessageQueue outgoing_message_queue;
  network::IncomingMessageQueue incoming_message_queue;

  outgoing_message_queue.AddRecipient(0);

  std::thread network_thread = network::client::Create(
      FLAGS_hostname.c_str(), FLAGS_port.c_str(),
      &outgoing_message_queue, &incoming_message_queue);

  SendGarbage(&outgoing_message_queue);

  network_thread.join();

  return 0;
}
