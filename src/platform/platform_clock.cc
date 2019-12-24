#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#include <cstdio>

#include "macro.h"
#include "rdtsc.h"

EXTERN(uint64_t median_tsc_per_usec);
EXTERN(uint64_t tsc_clock);

#define USEC_PER_CLOCK (1000.f / 1000.f / CLOCKS_PER_SEC)
#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)

int
cmp(const void* lhs, const void* rhs)
{
  uint64_t lhv = *((const uint64_t*)lhs);
  uint64_t rhv = *((const uint64_t*)rhs);
  if (lhv < rhv) return -1;
  if (lhv > rhv) return 1;
  return 0;
}

namespace platform
{
void
clock_init()
{
  clock_t c = clock();
  clock_t p;
  uint64_t rc = rdtsc();
  uint64_t rp;

#define MAX_SAMPLES 10
  uint64_t tsc_per_usec[MAX_SAMPLES];
  for (int i = 0; i < MAX_SAMPLES; ++i) {
    p = c;
    rp = rc;
    do {
      // clock measures time spent in the program (NB: excludes kernel)
      // a tight loop will push the scheduler to spend time in the application
      c = clock();
    } while (c - p < CLOCKS_PER_MS);
    rc = rdtsc();

    tsc_per_usec[i] = (rc - rp) * ((c - p) * USEC_PER_CLOCK);
  }

  qsort(tsc_per_usec, MAX_SAMPLES, sizeof(tsc_per_usec[0]), cmp);
  median_tsc_per_usec = tsc_per_usec[MAX_SAMPLES / 2];
  tsc_clock = rdtsc();
}

bool
elapse_usec(uint64_t frame_goal_usec, uint64_t* optional_sleep_usec,
            uint64_t* jerk)
{
  assert(median_tsc_per_usec);
  uint64_t tsc_now = rdtsc();
  uint64_t tsc_previous = tsc_clock;
  uint64_t tsc_step = frame_goal_usec * median_tsc_per_usec;
  uint64_t tsc_next = tsc_previous + tsc_step;

  if (tsc_next - tsc_now < tsc_step) {
    // no-op, busy wait
    // optional sleep time
    *optional_sleep_usec = (tsc_next - tsc_now) / median_tsc_per_usec;
    return false;
  } else if (tsc_now - tsc_next <= tsc_step) {
    // frame slightly over goal time
    // trivial advancement continues
    // within tsc_step do not sleep
    tsc_clock = tsc_next;
  } else if (tsc_now - tsc_next > tsc_step) {
    // non-trivial time-jerk, due to hardware clock or massive perf issue
    // do not sleep
    tsc_clock = tsc_now;
    *jerk += 1;
  }

  return true;
}
}  // namespace platform
