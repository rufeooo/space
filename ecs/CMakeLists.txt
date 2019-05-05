set(ecs_test_srcs
    "ecs.h"
    "ecs_test.cc")

add_executable(ecs_test ${ecs_test_srcs})
target_link_libraries(ecs_test Catch2::Catch2)
