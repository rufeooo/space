set(glfw_srcs
    "main.cc"
    "${CMAKE_SOURCE_DIR}/third_party/glfw/deps/glad_gl.c")

include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/deps)
include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/include)
add_executable(glfw_example ${glfw_srcs})
target_link_libraries(glfw_example glfw)
