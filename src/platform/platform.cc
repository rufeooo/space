#pragma once

#include "platform/platform.h"

#include "x64_intrin.h"

#include "platform_clock.cc"


#if _WIN32
#include "win32_window.cc"
#include "win32_sleep.cc"
//#include "win32_thread.cc"
//#include "win32_udp.cc"
#else
#include "unix/unix.cc"
#endif

//#include "affinity.cc"
