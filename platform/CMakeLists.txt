if (WIN32)
  set(filesystem_srcs
      "filesystem.h"
      "win32_filesystem.cc")
else (WIN32)
  set(filesystem_srcs
      "filesystem.h"
      "unix_filesystem.cc")
endif (WIN32)

add_library(filesystem ${filesystem_srcs})
