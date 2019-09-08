set(gtest_srcs
    "main_test.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)

add_executable(gtest_test ${gtest_srcs})
target_link_libraries(gtest_test gtest gmock)
set_target_properties(gtest_test PROPERTIES FOLDER Examples)
