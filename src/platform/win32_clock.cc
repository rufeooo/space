#include "sysinfoapi.h"

namespace platform
{
uint64_t
now_ns()
{
  FILETIME time;
  GetSystemTimeAsFileTime(&time);
  return ((uint64_t)(time.dwHighDateTime) << 32) | (uint64_t)time.dwLowDateTime;
}
}  // namespace platform
