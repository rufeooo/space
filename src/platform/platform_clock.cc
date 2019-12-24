#include <cstdint>
#include <ctime>

#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)

namespace platform
{
long
now_ns()
{
  timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  // Trivial, portable time implementation but the cost per call high due to system call
  return (time.tv_sec * 1000000000) + time.tv_nsec;
}
}  // namespace platform
