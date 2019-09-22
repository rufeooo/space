set(vec_test_srcs
    "vec.h"
    "vec_test.cc")

set(mat_test_srcs
    "mat.h"
    "mat_test.cc")

set(mat_ops_test_srcs
    "mat.h"
    "mat_ops.h"
    "mat_ops_test.cc")

set(quat_test_srcs
    "quat.h"
    "quat_test.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)

add_executable(vec_test ${vec_test_srcs})
target_link_libraries(vec_test gtest)
set_target_properties(vec_test PROPERTIES FOLDER Math)

add_executable(mat_test ${mat_test_srcs})
target_link_libraries(mat_test gtest)
set_target_properties(mat_test PROPERTIES FOLDER Math)

add_executable(mat_ops_test ${mat_ops_test_srcs})
target_link_libraries(mat_ops_test gtest)
set_target_properties(mat_ops_test PROPERTIES FOLDER Math)

add_executable(quat_test ${quat_test_srcs})
target_link_libraries(quat_test gtest)
set_target_properties(quat_test PROPERTIES FOLDER Math)
