#include "event_buffer.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>

namespace game {

namespace {

static uint8_t* kBuffer = nullptr;
static int kBufferSize = 0;
static int kIdx = 0;
static int kPollIdx = 0;

}

void AllocateEventBuffer(int size_bytes) {
  if (kBuffer) DeallocateEventBuffer();
  kBuffer = (uint8_t*)calloc(size_bytes, sizeof(uint8_t));
  kBufferSize = size_bytes;
}

void DeallocateEventBuffer() {
  free(kBuffer);
  kBufferSize = 0;
  kIdx = 0;
  kPollIdx = 0;
}

void* EnqueueEvent(uint16_t size, uint16_t metadata) {
  assert(kIdx + size < kBufferSize);
  *((uint16_t*)(kBuffer + kIdx)) = size; kIdx += 2;
  *((uint16_t*)(kBuffer + kIdx)) = metadata; kIdx += 2;
  uint8_t* data = (kBuffer + kIdx);
  kIdx += size;
  return data;
}

bool PollEvent(Event* event) {
  assert(event != nullptr);
  if (kPollIdx >= kIdx) return false;
  event->size = *((uint16_t*)(kBuffer + kPollIdx)); kPollIdx += 2;
  event->metadata = *((uint16_t*)(kBuffer + kPollIdx)); kPollIdx += 2;
  event->data = (kBuffer + kPollIdx);
  kPollIdx += event->size;
  return true;
}

void ResetEventBuffer() {
  memset(kBuffer, 0, kIdx);
  kIdx = 0;
  kPollIdx = 0;
}

}
