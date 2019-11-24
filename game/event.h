#pragma once

#include <cstdint>

namespace game {

struct Event {
  // Size of the event in bytes.
  uint16_t size;
  // meatadata used to hep determine the type.
  uint16_t metadata;
  // A pointer to the beginning of the event in the event_buffer.
  uint8_t* data = nullptr;
};

struct SavedEvent {
  // The game loop the event was run on.
  uint64_t game_loop;
  // The event that was run on the game loop.
  Event event;
};

}
