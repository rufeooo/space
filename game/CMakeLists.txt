set(game_test_srcs
    "game.h"
    "game.cc"
    "game_test.cc")

set(game_srcs
    "game.h"
    "game.cc")

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)

add_library(game_lib ${game_srcs})

add_executable(game_test ${game_test_srcs})
target_link_libraries(game_test gtest)
set_target_properties(game_test PROPERTIES FOLDER Game)
