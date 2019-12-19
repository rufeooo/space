include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)

include_directories(${CMAKE_SOURCE_DIR}/third_party/glad)
include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/gflags)

add_executable(space "space.cc" )

target_link_libraries(space glfw glad)
add_executable(asteroids_client "asteroids_client.cc")
target_link_libraries(asteroids_client glfw glad gflags)
add_executable(asteroids_server "asteroids_server.cc")
target_link_libraries(asteroids_server glfw glad gflags)
