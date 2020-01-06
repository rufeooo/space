#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#include "macro.h"
#include "rdtsc.h"

struct Clock_t {
  // Conversion constants
  uint64_t median_tsc_per_usec;
  uint64_t median_usec_per_tsc;
  // The ideal advacement cadence of tsc_clock
  uint64_t tsc_step;
  // Count the time rhythmic progression was lost
  uint64_t jerk;
  // Rhythmic time progression
  uint64_t tsc_clock;
  // Used for time delta within a single frame
  uint64_t frame_to_frame_tsc;
};

#define USEC_PER_CLOCK (1000.f / 1000.f / CLOCKS_PER_SEC)
#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)

int
cmp(const void *lhs, const void *rhs)
{
  uint64_t lhv = *((const uint64_t *)lhs);
  uint64_t rhv = *((const uint64_t *)rhs);
  if (lhv < rhv) return -1;
  if (lhv > rhv) return 1;
  return 0;
}

namespace platform
{
void
clock_init(uint64_t frame_goal_usec, Clock_t *out_clock)
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
  out_clock->median_tsc_per_usec = tsc_per_usec[MAX_SAMPLES / 2];

  // Calculate the step
  out_clock->tsc_step = frame_goal_usec * out_clock->median_tsc_per_usec;
  out_clock->median_usec_per_tsc =
      ((uint64_t)1 << 33) / out_clock->median_tsc_per_usec;
  // Init to current time
  out_clock->tsc_clock = rdtsc();
  out_clock->jerk = 0;
}

uint64_t
tscdelta_to_usec(const Clock_t *clock, uint64_t tsc)
{
  return (tsc * clock->median_usec_per_tsc) >> 33;
}

uint64_t
delta_usec(const Clock_t *clock)
{
  return tscdelta_to_usec(clock, rdtsc() - clock->frame_to_frame_tsc);
}

bool
elapse_usec(Clock_t *clock, uint64_t *optional_sleep_usec)
{
  uint64_t tsc_now = rdtsc();
  uint64_t tsc_previous = clock->tsc_clock;
  uint64_t tsc_next = tsc_previous + clock->tsc_step;

  if (tsc_next - tsc_now < clock->tsc_step) {
    // no-op, busy wait
    // optional sleep time
    *optional_sleep_usec = tscdelta_to_usec(clock, tsc_next - tsc_now);
    return false;
  } else if (tsc_now - tsc_next <= clock->tsc_step) {
    // frame slightly over goal time
    // trivial advancement continues
    // within tsc_step do not sleep
    clock->tsc_clock = tsc_next;
  } else if (tsc_now - tsc_next > clock->tsc_step) {
    // non-trivial time-jerk, due to hardware clock or massive perf issue
    // do not sleep
    clock->tsc_clock = tsc_now;
    clock->jerk += 1;
  }

  clock->frame_to_frame_tsc = tsc_now;
  return true;
}
}  // namespace platform
