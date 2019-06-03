set(game_test_srcs
    "game.h"
    "game.cc"
    "game_test.cc")

add_executable(game_test ${game_test_srcs})
target_link_libraries(game_test Catch2::Catch2)
