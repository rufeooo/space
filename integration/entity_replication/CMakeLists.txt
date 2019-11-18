set(entity_replication_srcs
    "entity_replication.h"
    "entity_replication.cc"
    "entity_replication_server.h"
    "entity_replication_server.cc")

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third_party/flatbuffers/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)

add_library(entity_replication ${entity_replication_srcs})

set(entity_replication_test_srcs "entity_replication_test.cc")

add_executable(entity_replication_test ${entity_replication_test_srcs})
target_link_libraries(entity_replication server client)
target_link_libraries(entity_replication_test entity_replication gtest)
