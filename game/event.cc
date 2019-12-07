#include "event.h"
#include "string.h"

namespace game {

Event Decode(uint8_t* msg) {
  Event e;
  memcpy(&e.size, &msg[0], sizeof(e.size));
  memcpy(&e.metadata, &msg[2], sizeof(e.metadata));
  e.data = &msg[4];
  return e;
}

}
