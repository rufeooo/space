set(glfw_renderer_srcs
    "renderer.h"
    "glfw_renderer.cc")

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/include)

add_library(glfw_renderer ${glfw_renderer_srcs})
target_link_libraries(glfw_renderer glfw glad)
set_target_properties(glfw_renderer PROPERTIES FOLDER renderer)
