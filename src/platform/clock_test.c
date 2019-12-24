#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)

#if defined(__i386__) || defined(__x86_64__)

uint64_t
rdtsc(void)
{
  return __builtin_ia32_rdtsc();
}

#else

#error "rdtsc() undefined for hardware"

#endif

int
cmp(const void* lhs, const void* rhs)
{
  uint64_t lhv = *((const uint64_t*)lhs);
  uint64_t rhv = *((const uint64_t*)rhs);
  if (lhv < rhv) return -1;
  if (lhv > rhv) return 1;
  return 0;
}

int
main()
{
  printf("%lu clocks_per_sec\n", CLOCKS_PER_SEC);
  printf("%lu clocks_per_ms\n", CLOCKS_PER_MS);

  clock_t c = clock();
  clock_t p;
  uint64_t rc = rdtsc();
  uint64_t rp;

#define MAX_SAMPLES 10
  uint64_t tsc_per_ms[MAX_SAMPLES];
  for (int i = 0; i < MAX_SAMPLES; ++i) {
    p = c;
    rp = rc;
    do {
      // clock measures time spent in the application
      // a tight loop will push the scheduler to spend time in the application
      c = clock();
    } while (c - p < CLOCKS_PER_MS);
    rc = rdtsc();

    tsc_per_ms[i] = (rc - rp) / (c - p);
    printf("%lu clock delta %lu tsc delta %lu tsc_per_ms\n", c - p, rc - rp,
           tsc_per_ms[i]);
  }

  qsort(tsc_per_ms, MAX_SAMPLES, sizeof(tsc_per_ms[0]), cmp);
  printf("median tsc_per_ms %lu\n", tsc_per_ms[MAX_SAMPLES / 2]);

  return 0;
}
