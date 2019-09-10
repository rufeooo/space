set(renderer_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR})
add_executable(renderer ${renderer_srcs})
target_link_libraries(renderer glfw_renderer)
set_target_properties(renderer PROPERTIES FOLDER Examples)
