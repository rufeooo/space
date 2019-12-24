
#pragma once

#include "platform_getopt.cc"
#include "window.cc"

#if _WIN32
#include "win32_clock.cc"
#include "win32_filesystem.cc"
#include "win32_udp.cc"
#include "win32_sleep.cc"
#else
#include "unix_clock.cc"
#include "unix_filesystem.cc"
#include "unix_udp.cc"
#include "unix_sleep.cc"
#endif
