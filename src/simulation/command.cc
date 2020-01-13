#pragma once

#include "common.cc"
#include "math/vec.h"

struct Command {
  enum Type {
    kNone = 0,
    kMine = 1,
    kMove = 2,
  };
  Type type;
  math::Vec2f destination;
};

DECLARE_GAME_TYPE(Command, 16);
