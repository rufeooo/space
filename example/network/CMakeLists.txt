
include_directories(${CMAKE_SOURCE_DIR}/third_party/gflags/src)

add_executable(network_udp_client "network_udp_client.cc")
target_link_libraries(network_udp_client  gflags)
