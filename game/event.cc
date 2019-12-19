#include "event.h"
#include "string.h"

#include <cassert>

namespace game
{
Event
Decode(uint8_t* msg)
{
  assert(msg != nullptr);
  Event e;
  memcpy(&e.size, &msg[0], sizeof(e.size));
  memcpy(&e.metadata, &msg[sizeof(e.size)], sizeof(e.metadata));
  e.data = &msg[kEventHeaderSize];
  return e;
}

void
Encode(uint16_t size, uint16_t metadata, const uint8_t* data, uint8_t* msg)
{
  assert(data != nullptr);
  memcpy(&msg[0], &size, sizeof(size));
  memcpy(&msg[sizeof(size)], &metadata, sizeof(metadata));
  memcpy(&msg[kEventHeaderSize], &data[0], size);
}

void
Build(uint16_t size, uint16_t metadata, const uint8_t* data,
      EventBuilder* builder)
{
  assert(builder != nullptr);
  assert(data != nullptr);
  assert(builder->idx + size < builder->size);
  memcpy(&builder->data[builder->idx], &size, sizeof(size));
  memcpy(&builder->data[builder->idx + sizeof(size)], &metadata,
         sizeof(metadata));
  memcpy(&builder->data[builder->idx + kEventHeaderSize], &data[0], size);
  builder->idx += size + kEventHeaderSize;
}

}  // namespace game
