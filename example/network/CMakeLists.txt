set(udp_client_srcs
    "network_udp_client.cc")


include_directories(${CMAKE_SOURCE_DIR}/third_party/gflags/src)
include_directories(${CMAKE_SOURCE_DIR})

add_executable(network_udp_client ${udp_client_srcs})
