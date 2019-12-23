#pragma once

#if _WIN32
#include "win32_window.cc"
#elif __APPLE__
#include "macosx_window.mm"
#elif __linux__
#include "x11_window.cc"
#endif
