set(search_test_srcs
    "search.h"
    "search_test.cc")

add_executable(search_test ${search_test_srcs})
target_link_libraries(search_test Catch2::Catch2)
set_target_properties(search_test PROPERTIES FOLDER Search)