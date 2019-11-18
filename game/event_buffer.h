#pragma once

#include "event.h"

namespace game {

// Initialize an event buffer of size_bytes 
void AllocateEventBuffer(int size_bytes);

// Free memory in event buffer.
void DeallocateEventBuffer();

// Reserve memory in the buffer, the return type should be
// casted to the desired type.
void* EnqueueEvent(uint16_t size, uint16_t metadata);

// Poll the event buffer until it's empty.
bool PollEvent(Event* event);

// Memset all memory in the buffer to 0.
void ResetEventBuffer();

}
