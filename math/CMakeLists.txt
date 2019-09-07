set(math_test_srcs
    "vec.h"
    "math_test.cc")

add_executable(math_test ${math_test_srcs})
target_link_libraries(math_test Catch2::Catch2)
set_target_properties(math_test PROPERTIES FOLDER Math)