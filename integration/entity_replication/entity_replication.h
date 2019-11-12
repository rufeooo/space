#pragma once

#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ecs/entity.h"

namespace integration {

struct EntitySystemDelta {
  // The id of the client who requires this delta.
  int client_id = -1;
  // Entities that need to be remove.
  std::vector<ecs::Entity> remove;
  // Entities that need to be added and an arbitrary byte stream
  // containing metadata to build the entity.
  std::vector<std::pair<ecs::Entity, std::vector<uint8_t>>> add;
};

// This class is reponsible for generating server -> client delta
// snapshots. When Run is called it will walk ALL client entities
// comparing them to the authority. A delta is constructed for each
// client with the containing the following -
//   
//    If the client contains an entity the server does not, a remove
//    will be added at the delta.
//    
//    If the server contains an entity the client does not an add
//    will be added with the data used to create that entity. 
class EntityReplication {
 public:
  // Calculate all deltas and store the result in deltas_.
  void Run();
  // Add entity to authority list with arbitrary byte stream, this
  // is likely the packet used to create the entity.
  void AddEntityAuthority(ecs::Entity, std::vector<uint8_t>&& data);
  // Drop the authorities entity.
  void RemoveEntityAuthority(ecs::Entity);

  // Add an entity that was created by the client.
  void AddEntityClient(int client_id, ecs::Entity);
  // Completely remove the client when they no longer need replication.
  void RemoveClient(int client_id);

  // Polls delta from the front of the deltas_ list. Returns true if
  // there was a delta to poll.
  bool PollDeltas(EntitySystemDelta* delta);

  // Apply the delta to existing state. This will add and remove
  // entities to the client list.
  void ApplyDelta(const EntitySystemDelta& delta);

 private:
  // The authority entity list is the entity list that should be
  // replicated to all clients.
  std::unordered_map<ecs::Entity, std::vector<uint8_t>>
      authority_entities_;
  // Client entity lists. A user assign an id to a client entity list
  // and EntitySystemDeltas will be made comparing authority_ to
  // client_.
  std::unordered_map<int, std::unordered_set<ecs::Entity>>
      client_entities_;
  // All deltas for every client.
  std::list<EntitySystemDelta> deltas_;
};

}
