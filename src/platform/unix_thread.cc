#include "thread.h"

#include <pthread.h>
#include <sched.h>
#include <cstddef>

static_assert(offsetof(ThreadInfo, id) == 0 &&
                  sizeof(ThreadInfo::id) >= sizeof(pthread_t),
              "ThreadInfo_t layout must be compatible with pthread_t");

typedef void* (*PThreadFunc)(void*);

namespace platform
{
bool
thread_create(ThreadInfo* t)
{
  if (t->id) return false;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create((pthread_t*)t, &attr, (PThreadFunc)t->func, t);

  return true;
}

void
thread_yield()
{
  sched_yield();
}

bool
thread_join(ThreadInfo* t)
{
  if (!t->id) return false;

  pthread_t* pt = (pthread_t*)t;
  pthread_join(*pt, 0);
  return true;
}

void
thread_exit(ThreadInfo* t, uint64_t value)
{
  t->return_value = value;
  pthread_exit(&t->return_value);
}

}  // namespace platform
