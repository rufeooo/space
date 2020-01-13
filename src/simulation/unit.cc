#pragma once

#include "common.cc"
#include "command.cc"

#include <cassert>

struct Unit {
  Transform transform;
  Command command;
  int kind = 0;
};

DECLARE_GAME_TYPE(Unit, 8);
