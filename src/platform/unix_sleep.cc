#include <unistd.h>
#include <cstdint>

namespace platform
{
void
sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}
void
sleep_usec(uint64_t usec)
{
  usleep(usec);
}

}  // namespace platform
