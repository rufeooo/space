#include <unistd.h>
#include <cstdint>

namespace platform
{
void
sleep_ms(uint64_t duration)
{
  usleep(duration * 1000);
}

}  // namespace platform
