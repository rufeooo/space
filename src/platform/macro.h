#pragma once

#ifdef __cplusplus
#define EXTERN(x) \
  extern "C" {    \
  x;              \
  }
#else
#define EXTERN(x) x
#endif

#define MIN(x, y) (y ^ ((x ^ y) & -(x < y)))
#define MAX(x, y) (x ^ ((x ^ y) & -(x < y)))
#define CLAMP(x, min, max) MIN(MAX(x, min), max)

#if _WIN32
#define ALIGN(n) alignas(n)
#else
#define ALIGN(n) __attribute__((aligned(n)))
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))
#endif

