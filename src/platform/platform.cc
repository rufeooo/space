
#pragma once

#include "platform_getopt.cc"
#include "window.cc"

#if _WIN32
#include "win32_filesystem.cc"
#include "win32_udp.cc"
#else
#include "unix_filesystem.cc"
#include "unix_udp.cc"
#endif
