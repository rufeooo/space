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

set(intersection_test_srcs
    "intersection.h"
    "intersection.cc"
    "intersection_test.cc")

set(math_lib_srcs
    "intersection.cc"
    "intersection.h"
    "mat.h"
    "mat_ops.h"
    "utils.h"
    "quat.h"
    "vec.h")
  

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

add_library(math_lib ${math_lib_srcs})

add_executable(intersection_test ${intersection_test_srcs})
target_link_libraries(intersection_test gtest math_lib)
set_target_properties(intersection_test PROPERTIES FOLDER Math)

