#include "thread.h"

// TODO: This only works for a single thread. The API is hard
// to make consistent with pthread for the following reasons -
//
// CreateThread returns a Handle to a thread. This is a win32
// specific object. The thread API assumes an id that the
// user can pick.
//
// The functor returns a DWORD and not a void*.

namespace platform {

struct Thread {
  HANDLE handle;
  DWORD thread_id;
  ThreadFunc thread_func;
  ThreadInfo thread_info;
};

static Thread kThread;

DWORD WINAPI Win32ThreadFunc(LPVOID lpParam) 
{
  kThread.thread_func(&kThread.thread_info);
  return 0;
}

bool
thread_create(ThreadInfo* t, ThreadFunc func)
{
  if (t->id) return false;

  kThread.thread_func = func;
  kThread.handle = CreateThread(
      NULL,
      0/* Default stack size */,
      Win32ThreadFunc,
      t->arg,
      0,
      &kThread.thread_id);

  return true;
}

void
thread_yield()
{
}

bool
thread_join(ThreadInfo* t)
{
  WaitForSingleObject(kThread.handle, INFINITE);
  return 0;
}

void
thread_exit(ThreadInfo* t, uint64_t value)
{
}

}
