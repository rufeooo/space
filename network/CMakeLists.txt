if (WIN32)
  set(network_srcs
      "network.h"
      "win32_network.cc")
else (WIN32)
  set(network_srcs
      "network.h"
      "unix_network.cc")
endif (WIN32)

add_library(network ${network_srcs})
