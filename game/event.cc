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

}
