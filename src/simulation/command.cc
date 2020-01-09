
#include "entity.cc"

namespace command
{
enum Event {
  INVALID = 0,
  MOVE = 1,
};

struct Move {
  uint64_t entity_id;
  math::Vec2f position;
};

void
Execute(const Move& move)
{
  kWriteEntity[move.entity_id].destination.position = move.position;
}

}  // namespace command
