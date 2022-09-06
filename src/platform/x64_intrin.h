#pragma once

#include <cstdint>

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#ifdef _WIN32
// TODO: enforce target arch?
#define TARGET(x)
#else
// target cpu requirement
#define TARGET(x) __attribute__((target(x)))
#endif

// Reset lowest set bit to 0
inline uint64_t TARGET("bmi") BLSR(uint64_t f)
{
  return _blsr_u64(f);
}

// Right 0s: Trailing zero count
inline uint64_t TARGET("bmi") TZCNT(uint64_t f)
{
  return _tzcnt_u64(f);
}

// Left 0s: Leading zero count
inline uint64_t TARGET("lzcnt") LZCNT(uint64_t f)
{
  return _lzcnt_u64(f);
}

// 1s Population count
inline uint64_t TARGET("popcnt") POPCNT(uint64_t f)
{
  return _mm_popcnt_u64(f);
}

// Bits in 'a' are cleared in 'f'
// Logically: ~a & f
inline uint64_t TARGET("bmi") ANDN(uint64_t a, uint64_t f)
{
  return _andn_u64(a, f);
}
