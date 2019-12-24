#include <cstdint>
#include <ctime>

namespace platform
{
long
now_ns()
{
  timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return (time.tv_sec * 1000000000) + time.tv_nsec;
}
}  // namespace platform
