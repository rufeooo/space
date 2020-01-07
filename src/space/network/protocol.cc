#pragma once

#include <cstdint>

#include "platform/platform.cc"

const uint64_t greeting_size = 8;
#define GREETING "spacehi"

struct Handshake {
  const char greeting[greeting_size] = {GREETING};
  uint64_t num_players;
};

struct NotifyStart {
  uint64_t game_id;
  uint64_t player_id;
  uint64_t player_count;
};

struct Turn {
  uint64_t sequence;
  uint64_t player_id;
  PlatformEvent event[];
};

struct NotifyTurn {
  uint64_t frame;
  uint64_t player_id;
  uint64_t ack_sequence;
  PlatformEvent event[];
};
