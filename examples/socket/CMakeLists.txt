if (UNIX)
  set(unix_list_srcs
      "unix_list.cc")

  add_executable(unix_list ${unix_list_srcs})
  set_target_properties(unix_list PROPERTIES FOLDER Examples)
endif (UNIX)

set(time_server_srcs
    "time_server.cc")

set(tcp_client_srcs
    "tcp_client.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/gflags/src)
include_directories(${CMAKE_SOURCE_DIR})

add_executable(time_server ${time_server_srcs})
target_link_libraries(time_server network)

add_executable(tcp_client ${tcp_client_srcs})
target_link_libraries(tcp_client network gflags)
