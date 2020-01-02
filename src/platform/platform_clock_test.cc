#include "platform.cc"

#include <cstdint>
#include <cstdio>

int
main(int argc, char** argv)
{
  uint64_t framerate = 30;
  int runtime_seconds = 1;

  while (1) {
    int opt = platform_getopt(argc, argv, "f:s:");

    if (opt == -1) break;

    switch (opt) {
      case 'f':
        framerate = atoi(platform_optarg);
        break;
      case 's':
        runtime_seconds = atoi(platform_optarg);
        break;
    }
  }

  int target_usec = 1000.0 * 1000.0 / framerate;
  Clock_t clock;
  uint64_t sleep_usec = 0;
  uint64_t frame = 0;
  uint64_t sleep_count = 0;
  uint64_t* delta =
      (uint64_t*)malloc(sizeof(uint64_t) * framerate * runtime_seconds);

  platform::clock_init(target_usec, &clock);

  while (frame < framerate * runtime_seconds) {
    delta[frame] = platform::delta_usec(&clock);
    while (!platform::elapse_usec(&clock, &sleep_usec)) {
      usleep(sleep_usec);
      ++sleep_count;
    }
    ++frame;
  }
  printf("[0x%lx tsc_clock] [ %lu sleep_usec ] [ %lu jerk ] [ %lu frame ]\n",
         clock.tsc_clock, sleep_usec, clock.jerk, frame);

  for (int i = 0; i < frame; ++i) {
    printf("%lu ", delta[i]);
  }
  puts("");

  printf("%lu sleep count, %lu frame\n", sleep_count, frame);

  return 0;
}
