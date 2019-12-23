
#pragma once

#include "window.cc"
#if _WIN32
#include "win32_filesystem.cc"
#else
#include "unix_filesystem.cc"
#endif
