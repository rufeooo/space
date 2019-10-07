set(asteroids_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR})

add_executable(asteroids ${asteroids_srcs})
target_link_libraries(asteroids
                      game_lib glfw_renderer server client math_lib
                      gflags)
set_target_properties(asteroids PROPERTIES FOLDER Demos)
