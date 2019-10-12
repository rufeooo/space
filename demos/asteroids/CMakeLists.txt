set(asteroids_srcs
    "main.cc")

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

add_executable(asteroids ${asteroids_srcs})
target_link_libraries(asteroids
                      game_lib glfw_renderer server client math_lib
                      gflags)
add_dependencies(asteroids asteroids_packet_fbs)
set_target_properties(asteroids PROPERTIES FOLDER Demos)
