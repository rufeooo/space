#include "entity_replication_server.h"

#include <cassert>
#include <iostream>
#include <thread>

#include "entity_replication.h"
#include "network/client.h"
#include "network/server.h"

namespace integration
{
namespace entity_replication
{
namespace
{
std::thread kNetworkThread;
EntityReplication kEntityReplication;
ReplicationType kReplicationType;

}  // namespace

void
Start(ReplicationType replication_type, const char* port, const char* hostname)
{
  kReplicationType = replication_type;
  switch (kReplicationType) {
    case ReplicationType::SERVER:
      break;
    case ReplicationType::CLIENT:
      break;
    default:
      assert("Unknown replication type.");
  }
}

void
Stop()
{
}

void
CreateEntity(ecs::Entity entity, std::vector<uint8_t>&& command_data)
{
  switch (kReplicationType) {
    case ReplicationType::SERVER:
      kEntityReplication.AddEntityAuthority(entity, std::move(command_data));
      break;
    case ReplicationType::CLIENT:
      break;
    default:
      assert("Unknown replication type.");
  }
}

void
RemoveEntity(ecs::Entity entity)
{
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

}  // namespace entity_replication

}  // namespace integration
