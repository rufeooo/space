#pragma once

#include "common.cc"

#include <cassert>

struct Unit {
  Transform transform;
  math::Vec2f destination;
  int kind = 0;
};

DECLARE_GAME_TYPE(Unit, 8);
