set(ecs_test_srcs
    "ecs.h"
    "ecs_test.cc")

add_executable(ecs_test ${ecs_test_srcs})
target_link_libraries(ecs_test Catch2::Catch2)
set_target_properties(ecs_test PROPERTIES FOLDER ECS)

set(system_test_srcs
    "ecs.h"
    "system.h"
    "system_test.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)

add_executable(system_test ${system_test_srcs})
target_link_libraries(system_test gtest gmock)
set_target_properties(system_test PROPERTIES FOLDER ECS)
