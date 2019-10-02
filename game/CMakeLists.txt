set(game_test_srcs
    "game.h"
    "game.cc"
    "game_test.cc")

set(game_srcs
    "game.h"
    "game.cc")

include_directories(${CMAKE_SOURCE_DIR})

add_library(game_lib ${game_srcs})

add_executable(game_test ${game_test_srcs})
set_target_properties(game_test PROPERTIES FOLDER Game)
