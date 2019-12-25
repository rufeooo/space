#pragma once

#include <cstdint>

#if defined(__i386__) || defined(__x86_64__)
#define rdtsc() __builtin_ia32_rdtsc()
#elif _WIN32
#include <intrin.h>
#define rdtsc() __rdtsc();
#else
#error "rdtsc() undefined for hardware"
#endif
