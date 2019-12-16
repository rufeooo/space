#pragma once

#include <cstdint>

#include "math/vec.h"

namespace command {

enum Event {
  INVALID  = 0,
  MOVE     = 1,
};

struct Move {
  uint64_t entity_id;
  math::Vec2f position;
};

void Execute(const Move& move);

}
