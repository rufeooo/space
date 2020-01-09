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
#define ALIGNAS(n) alignas(n)
#else
// ALIGN macro taken in macos.
#define ALIGNAS(n) __attribute__((aligned(PAGE)))
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))
#endif

