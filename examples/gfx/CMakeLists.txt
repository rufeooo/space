set(gl_version_srcs
    "gl_version.cc"
    "${CMAKE_SOURCE_DIR}/third_party/glfw/deps/glad_gl.c")

include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/deps)
include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/include)
add_executable(gl_version ${gl_version_srcs})
target_link_libraries(gl_version glfw)
