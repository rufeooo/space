
#include "unit.cc"

namespace command
{
enum Event {
  INVALID = 0,
  MOVE = 1,
};

struct Move {
  uint64_t unit_id;
  math::Vec2f position;
};

void
Execute(const Move& move)
{
  kUnit[move.unit_id].destination = move.position;
}

}  // namespace command
