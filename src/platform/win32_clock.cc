#include "sysinfoapi.h"

namespace platform
{
uint64_t
now_ns()
{
  __int64 wintime;
  // Gives 100ns precision.
  GetSystemTimeAsFileTime((FILETIME*)&wintime);
  // Convert jan 1 1601 to jan 1 1970.
  wintime -= 116444736000000000i64;
  // Get windows seconds, convert it to nanoseconds and add nanoseconds.
  // This makes this quall functionally equiavlent to clock_gettime on unix. See unix_clock.cc
  return (wintime / 10000000i64) * 1000000000 + (wintime % 10000000i64 * 100);
}
}  // namespace platform
