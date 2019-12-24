#include <cstdint>
#include <ctime>

#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)

namespace platform
{
uint64_t
now_ms()
{
  // Trivial, portable time implementation but the cost per call high due to system call
  return clock() / CLOCKS_PER_MS;
}
}  // namespace platform
