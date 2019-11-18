#include "entity_replication_server.h"

#include <cassert>
#include <iostream>

#include "entity_replication.h"
#include "network/server.h"
#include "network/client.h"
#include "network/message_queue.h"


namespace integration {

namespace entity_replication {

namespace {

network::OutgoingMessageQueue kOutgoingMessageQueue;
network::IncomingMessageQueue kIncomingMessageQueue;
std::thread kNetworkThread;
EntityReplication kEntityReplication;
ReplicationType kReplicationType;

}  // namespace

void Start(ReplicationType replication_type,
           const char* port, const char* hostname) {
  kReplicationType = replication_type;
  switch (kReplicationType) {
    case ReplicationType::SERVER:
      kNetworkThread = network::server::Create(
          port, &kOutgoingMessageQueue, &kIncomingMessageQueue);
      break;
    case ReplicationType::CLIENT:
      kNetworkThread = network::client::Create(
          hostname, port, &kOutgoingMessageQueue, &kIncomingMessageQueue);
      break;
    default:
      assert("Unknown replication type.");
  }
}

void Stop() {
  if (kNetworkThread.joinable()) {
    kIncomingMessageQueue.Stop();
    kNetworkThread.join();
  }
}

void CreateEntity(ecs::Entity entity, std::vector<uint8_t>&&
      command_data) {
  switch (kReplicationType) {
    case ReplicationType::SERVER:
      kEntityReplication.AddEntityAuthority(
          entity, std::move(command_data));
      break;
    case ReplicationType::CLIENT:
      break;
    default:
      assert("Unknown replication type.");
  }
}

void RemoveEntity(ecs::Entity entity) {
  switch (kReplicationType) {
    case ReplicationType::SERVER:
      kEntityReplication.RemoveEntityAuthority(entity);
      break;
    case ReplicationType::CLIENT:
      break;
    default:
      assert("Unknown replication type.");
  }
}


}  // entity_replication

}  // integration
