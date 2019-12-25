#include "command.h"

#include "ecs.cc"

namespace command
{
void
Execute(const Move& move)
{
  auto* dest = kECS.Assign<DestinationComponent>(move.entity_id);
  dest->position = move.position;
}

}  // namespace command
