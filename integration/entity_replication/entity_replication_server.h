#pragma once

#include "ecs/entity.h"

#include <functional>
#include <vector>

namespace integration
{
namespace entity_replication
{
enum class ReplicationType { SERVER = 0, CLIENT = 1 };

// Spins up a server or client connection
void Start(ReplicationType replication_type, const char* port = "9456",
           const char* hostname = "127.0.0.1");

void Stop();

// Inform the replication system an entity was created.
//
// If the replication system is the SERVER run the
// serialize_functor and store off the create command.
//
// If the replication system is set to be a CLIENT
// simply forward command_data to the SERVER.
void CreateEntity(ecs::Entity entity, std::vector<uint8_t>&& command_data);

// Infrom the replication system an entity was deleted.
void RemoveEntity(ecs::Entity);

}  // namespace entity_replication

}  // namespace integration
