set(glfw_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/deps)
include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/include)
add_executable(glfw_example ${glfw_srcs})
target_link_libraries(glfw_example glfw)
