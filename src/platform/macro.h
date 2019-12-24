#pragma once

#ifdef __cplusplus
#define EXTERN(x) \
  extern "C" {    \
  x;              \
  }
#else
#define EXTERN(x) x
#endif
