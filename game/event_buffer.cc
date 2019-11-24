#include "event_buffer.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iostream>

namespace game {

namespace {

static EventBuffer kDefaultEventBuffer;
static EventBuffer* kCustomEventBuffer = nullptr;

}

void AllocateEventBuffer(int size_bytes) {
  auto& events = kDefaultEventBuffer; 
  if (events.buffer) DeallocateEventBuffer();
  events.buffer = (uint8_t*)calloc(size_bytes, sizeof(uint8_t));
  events.buffer_size = size_bytes;
}

void DeallocateEventBuffer() {
  auto& events = kDefaultEventBuffer; 
  free(events.buffer);
  events.buffer_size = 0;
  events.idx = 0;
  events.poll_idx = 0;
}

void* EnqueueEvent(uint16_t size, uint16_t metadata) {
  auto& events = kDefaultEventBuffer; 
  assert(events.idx + size < events.buffer_size);
  *((uint16_t*)(events.buffer + events.idx)) = size;
  events.idx += 2;
  *((uint16_t*)(events.buffer + events.idx)) = metadata;
  events.idx += 2;
  uint8_t* data = (events.buffer + events.idx);
  events.idx += size;
  return data;
}

bool PollEvent(Event* event) {
  assert(event != nullptr);
  // Prefer polling events from a custom event buffer.
  auto& events = kCustomEventBuffer != nullptr ?
      *kCustomEventBuffer : kDefaultEventBuffer; 
  if (events.poll_idx >= events.idx) return false;
  event->size = *((uint16_t*)(events.buffer + events.poll_idx));
  events.poll_idx += 2;
  event->metadata = *((uint16_t*)(events.buffer + events.poll_idx));
  events.poll_idx += 2;
  event->data = (events.buffer + events.poll_idx);
  events.poll_idx += event->size;
  return true;
}

void ResetEventBuffer() {
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

void SetCustomEventBuffer(EventBuffer* event_buffer) {
  kCustomEventBuffer = event_buffer;
}

}
