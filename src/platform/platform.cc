#pragma once

#if _WIN32
// Windows #defines min/max. This stops it from doing so.
#define NOMINMAX
#endif

#include "platform_getopt.cc"
#include "platform_clock.cc"
#include "window.cc"

#if _WIN32
#include "win32_filesystem.cc"
#include "win32_udp.cc"
#include "win32_sleep.cc"
#else
#include "unix_filesystem.cc"
#include "unix_udp.cc"
#include "unix_sleep.cc"
#endif
