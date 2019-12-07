set(search_test_srcs
    "search.h"
    "search_test.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)

add_executable(search_test ${search_test_srcs})
target_link_libraries(search_test gtest)
set_target_properties(search_test PROPERTIES FOLDER Search)
