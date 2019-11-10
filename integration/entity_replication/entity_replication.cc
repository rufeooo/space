#include "entity_replication.h"

#include <cassert>
#include <iostream>

namespace integration {

void EntityReplication::Run() {
  // Create the EntitySystemDelta for each client. 
  std::unordered_map<int, EntitySystemDelta> client_deltas;
  for (auto& c : client_entities_) {
    client_deltas[c.first] = EntitySystemDelta();
  }
  // Walk all server entities comparing them to each client.
  for (const auto& authority : authority_entities_) {
    for (const auto& client : client_entities_) {
      auto authority_on_client = client.second.find(authority.first);
      if (authority_on_client == client.second.end()) {
        EntitySystemDelta& delta = client_deltas[client.first];
        delta.client_id = client.first;
        delta.add.emplace_back(authority.first, authority.second);
      }
    }
  }
  // Walk all client entities comparing them to the authority
  for (const auto& client : client_entities_) {
    auto& client_entites = client.second;
    for (const auto& client_entity : client_entites) {
      auto found = authority_entities_.find(client_entity);
      if (found != authority_entities_.end()) continue;
      EntitySystemDelta& delta = client_deltas[client.first];
      delta.client_id = client.first;
      delta.remove.push_back(client_entity);
    }
  }
  for (auto& delta : client_deltas) {
    if (delta.second.client_id == -1) continue; // No delta
    deltas_.push_back(std::move(delta.second));
  }
}

void EntityReplication::AddEntityAuthority(
    ecs::Entity entity, std::vector<uint8_t>&& data) {
  authority_entities_[entity] = data;
}

void EntityReplication::RemoveEntityAuthority(ecs::Entity entity) {
  auto found = authority_entities_.find(entity);
  if (found == authority_entities_.end()) return;
  authority_entities_.erase(found);
}

void EntityReplication::AddEntityClient(
    int client_id, ecs::Entity entity) {
  assert(client_id != -1);
  auto& client_set = client_entities_[client_id];
  client_set.insert(entity);
}

void EntityReplication::RemoveClient(int client_id) {
  auto found = client_entities_.find(client_id);
  if (found == client_entities_.end()) return;
  client_entities_.erase(found);
}

bool EntityReplication::PollDeltas(EntitySystemDelta* delta) {
  assert(delta != nullptr);
  if (deltas_.empty()) {
    return false;
  }
  *delta = std::move(deltas_.front());
  deltas_.pop_front();
  return true;
}


}
