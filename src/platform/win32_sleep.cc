#include <cstdint>
#include "synchapi.h"

namespace platform
{
void
sleep_ms(uint64_t duration)
{
  Sleep(duration);
}
}  // namespace platform
