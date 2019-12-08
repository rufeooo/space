#include "event.h"
#include "string.h"

namespace game {

Event Decode(uint8_t* msg) {
  Event e;
  memcpy(&e.size, &msg[0], sizeof(e.size));
  memcpy(&e.metadata, &msg[sizeof(e.size)], sizeof(e.metadata));
  e.data = &msg[kEventHeaderSize];
  return e;
}

void Encode(uint16_t size, uint16_t metadata,
            const uint8_t* data, uint8_t* msg) {
  memcpy(&msg[0], &size, sizeof(size));
  memcpy(&msg[sizeof(size)], &metadata, sizeof(metadata));
  memcpy(&msg[kEventHeaderSize], &data[0], size);
}

}
