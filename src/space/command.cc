#include "command.h"

#include "entity.cc"

namespace command
{
void
Execute(const Move& move)
{
  game_entity[move.entity_id].destination.position = move.position;
}

}  // namespace command
