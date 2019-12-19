
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third_party/glad)
include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/include)

add_library(glfw_renderer "renderer.cc")
target_link_libraries(glfw_renderer glfw glad math_lib)
set_target_properties(glfw_renderer PROPERTIES FOLDER renderer)

include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)

add_executable(gl_shader_cache_test "gl_shader_cache_test.cc")
target_link_libraries(gl_shader_cache_test gtest gmock glfw glad)
set_target_properties(gl_shader_cache_test PROPERTIES FOLDER renderer)
