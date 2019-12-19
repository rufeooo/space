#include "event_buffer.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <mutex>

#include "event.h"

namespace game
{
namespace
{
static EventBuffer kDefaultEventBuffer;
static EventBuffer* kCustomEventBuffer = nullptr;
static std::mutex kMutex;

}  // namespace

void
AllocateEventBuffer(int size_bytes)
{
  auto& events = kDefaultEventBuffer;
  if (events.buffer) DeallocateEventBuffer();
  events.buffer = (uint8_t*)calloc(size_bytes, sizeof(uint8_t));
  events.buffer_size = size_bytes;
}

void
DeallocateEventBuffer()
{
  auto& events = kDefaultEventBuffer;
  free(events.buffer);
  events.buffer_size = 0;
  events.idx = 0;
  events.poll_idx = 0;
}

void*
EnqueueEvent(uint16_t size, uint16_t metadata)
{
  std::lock_guard<std::mutex> guard(kMutex);
  assert(size != 0);
  auto& events = kDefaultEventBuffer;
  assert(events.idx + size < events.buffer_size);
  *((uint16_t*)(events.buffer + events.idx)) = size;
  events.idx += sizeof(size);
  *((uint16_t*)(events.buffer + events.idx)) = metadata;
  events.idx += sizeof(metadata);
  uint8_t* data = (events.buffer + events.idx);
  events.idx += size;
  return data;
}

void
EnqueueEvent(uint8_t* event, int size)
{
  std::lock_guard<std::mutex> guard(kMutex);
  assert(size != 0);
  auto& events = kDefaultEventBuffer;
  assert(events.idx + size < events.buffer_size);
  memcpy(&events.buffer[events.idx], event, size);
  events.idx += size;
}

bool
PollEvent(Event* event)
{
  std::lock_guard<std::mutex> guard(kMutex);
  assert(event != nullptr);
  // Prefer polling events from a custom event buffer.
  auto& events =
      kCustomEventBuffer != nullptr ? *kCustomEventBuffer : kDefaultEventBuffer;
  if (events.poll_idx >= events.idx) return false;
  *event = Decode(&events.buffer[events.poll_idx]);
  // This is bad. A message of 0 size should never make its
  // way in or out of an event queue.
  assert(event->size != 0);
  events.poll_idx +=
      sizeof(event->size) + sizeof(event->metadata) + event->size;
  return true;
}

void
ResetEventBuffer()
{
  std::lock_guard<std::mutex> guard(kMutex);
  auto& events = kDefaultEventBuffer;
  memset(events.buffer, 0, events.idx);
  events.idx = 0;
  events.poll_idx = 0;
  if (kCustomEventBuffer) {
    auto& custom_events = *kCustomEventBuffer;
    memset(custom_events.buffer, 0, custom_events.idx);
    custom_events.idx = 0;
    custom_events.poll_idx = 0;
  }
}

void
SetCustomEventBuffer(EventBuffer* event_buffer)
{
  std::lock_guard<std::mutex> guard(kMutex);
  kCustomEventBuffer = event_buffer;
}

}  // namespace game
