set(asteroids_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR})

add_executable(asteroids ${asteroids_srcs})
target_link_libraries(asteroids sdl_game_lib)
set_target_properties(asteroids PROPERTIES FOLDER Demos)