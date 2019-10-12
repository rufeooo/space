include_directories(${CMAKE_SOURCE_DIR})

build_flatbuffers(
    "protocol/asteroids_packet.fbs" "" asteroids_packet_fbs "" "protocol/" "" "")

add_library(asteroids_lib "asteroids.h" "asteroids.cc")
add_executable(asteroids_client "asteroids_client.cc")
add_executable(asteroids_server "asteroids_server.cc")

target_link_libraries(asteroids_lib glfw_renderer math_lib)
target_link_libraries(asteroids_client asteroids_lib game_lib
                      glfw_renderer client gflags)
target_link_libraries(asteroids_server asteroids_lib game_lib
                      glfw_renderer server gflags)

set_target_properties(asteroids_lib PROPERTIES FOLDER Demos)
set_target_properties(asteroids_client PROPERTIES FOLDER Demos)
set_target_properties(asteroids_server PROPERTIES FOLDER Demos)
