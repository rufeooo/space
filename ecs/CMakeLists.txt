set(ecs_test_srcs
    "ecs.h"
    "ecs_test.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)

add_executable(ecs_test ${ecs_test_srcs})
target_link_libraries(ecs_test gtest)
set_target_properties(ecs_test PROPERTIES FOLDER ECS)
