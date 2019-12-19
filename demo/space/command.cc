#include "command.h"

#include "components.h"
#include "components/common/transform_component.h"
#include "ecs.h"

namespace command
{
void
Execute(const Move& move)
{
  auto* dest = kECS.Assign<DestinationComponent>(move.entity_id);
  dest->position = move.position;
}

}  // namespace command
