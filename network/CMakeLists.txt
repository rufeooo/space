if (WIN32)
  set(network_srcs
      "network.h"
      "win32_network.cc")
else (WIN32)
  set(network_srcs
      "network.h"
      "unix_network.cc")
endif (WIN32)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)

build_flatbuffers(
    "testdata/packet.fbs" "" testdata_packet_fbs "" "testdata/" "" "")

add_library(network ${network_srcs})

set(message_queue_srcs
    "message_queue.h"
    "message_queue.cc")

add_library(message_queue ${message_queue_srcs})
target_link_libraries(message_queue flatbuffers)

set(server_srcs
    "server.h"
    "server.cc")

add_library(server ${server_srcs})
target_link_libraries(server message_queue network)
add_dependencies(server testdata_packet_fbs)

set(server_test_srcs
    "server_test.cc")

add_executable(server_test ${server_test_srcs})
target_link_libraries(server_test server gtest)

set(client_srcs
    "client.h"
    "client.cc")

add_library(client ${client_srcs})
target_link_libraries(client message_queue network)

set(client_test_srcs
    "client_test.cc")

add_executable(client_test ${client_test_srcs})
target_link_libraries(client_test server client gtest)
