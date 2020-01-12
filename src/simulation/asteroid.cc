#pragma once

#include "common.cc"

#include <cassert>

struct Asteroid {
  Transform transform;
};

DECLARE_GAME_TYPE(Asteroid, 8);
