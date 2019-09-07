set(game_test_srcs
    "game.h"
    "game.cc"
    "game_test.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/SDL-mirror/include)
add_library(sdl_game_lib
  game.h game.cc sdl_game.h sdl_game.cc)
target_link_libraries(sdl_game_lib SDL2)

add_executable(game_test ${game_test_srcs})
target_link_libraries(game_test Catch2::Catch2 sdl_game_lib)
set_target_properties(sdl_game_lib PROPERTIES FOLDER Game)
set_target_properties(game_test PROPERTIES FOLDER Game)