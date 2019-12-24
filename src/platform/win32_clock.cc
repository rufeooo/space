#include "sysinfoapi.h"

namespace platform
{
uint64_t
now_ns()
{
  FILETIME time;
  GetSystemTimeAsFileTime(&time);
  // TODO: Not sure this is correct. Windows behaves quicker than I'd expect it to.
  return (((uint64_t)(time.dwHighDateTime) << 32) | (uint64_t)time.dwLowDateTime) * 1000;
}
}  // namespace platform
