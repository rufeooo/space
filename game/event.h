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

constexpr int kEventHeaderSize
    = sizeof(Event::size) + sizeof(Event::metadata);

struct SavedEvent {
  // The game loop the event was run on.
  uint64_t game_loop;
  // The event that was run on the game loop.
  Event event;
};

// Converts buffer and size to Event struct. And moves msg
// forward to next event.
Event Decode(uint8_t* msg);

// Copy data into msg given the size and metadata.
void Encode(uint16_t size, uint16_t metadata,
            const uint8_t* data, uint8_t* msg);

}
