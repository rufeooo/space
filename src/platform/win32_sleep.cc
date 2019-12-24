#include <cstdint>
#include "synchapi.h"

namespace platform
{
void
sleep_ms(uint64_t duration)
{
  Sleep(duration);
}
void
sleep_usec(uint64_t usec)
{
  // TODO: Yield
}
}  // namespace platform
