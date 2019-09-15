set(vec_test_srcs
    "vec.h"
    "vec_test.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)

add_executable(vec_test ${vec_test_srcs})
target_link_libraries(vec_test gtest)
set_target_properties(vec_test PROPERTIES FOLDER Math)
