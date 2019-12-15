include_directories(${CMAKE_SOURCE_DIR})

add_executable(space "space.cc")

target_link_libraries(space glfw_renderer math_lib game_lib)
