#pragma once

#include <cstdint>

struct ThreadInfo {
  uint64_t id;
  void* arg;
  uint64_t return_value;
} ti;

typedef void* (*ThreadFunc)(ThreadInfo*);

namespace platform
{
bool thread_create(ThreadInfo* t, ThreadFunc func);
void thread_yield();
bool thread_join(ThreadInfo* t);
void thread_exit(ThreadInfo* t, uint64_t value);
}  // namespace platform
