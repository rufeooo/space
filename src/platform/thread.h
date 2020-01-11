#pragma once

#include <cstdint>

struct ThreadInfo;
typedef void* (*ThreadFunc)(const ThreadInfo*);

struct ThreadInfo {
  uint64_t id;
  ThreadFunc func;
  void* arg;
  uint64_t return_value;
} ti;

namespace platform
{
bool thread_create(ThreadInfo* t);
void thread_yield();
bool thread_join(ThreadInfo* t);
void thread_exit(ThreadInfo* t, uint64_t value);
}  // namespace platform
