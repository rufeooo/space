set(asteroids_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR})

build_flatbuffers(
    "protocol/asteroids_packet.fbs" "" asteroids_packet_fbs "" "protocol/" "" "")

add_executable(asteroids ${asteroids_srcs})
target_link_libraries(asteroids
                      game_lib glfw_renderer server client math_lib
                      gflags)
add_dependencies(asteroids asteroids_packet_fbs)
set_target_properties(asteroids PROPERTIES FOLDER Demos)
