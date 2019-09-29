set(glfw_renderer_srcs
    "shader_cache.h"
    "gl_shader_cache.h"
    "gl_shader_cache.cc"
    "gl_utils.h"
    "gl_utils.cc")

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third_party/glad)
include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/include)

add_library(glfw_renderer ${glfw_renderer_srcs})
target_link_libraries(glfw_renderer glfw glad math_lib)
set_target_properties(glfw_renderer PROPERTIES FOLDER renderer)

set(gl_shader_cache_test_srcs
    "shader_cache.h"
    "gl_shader_cache.h"
    "gl_shader_cache.cc"
    "gl_shader_cache_test.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)

add_executable(gl_shader_cache_test ${gl_shader_cache_test_srcs})
target_link_libraries(gl_shader_cache_test gtest gmock glfw_renderer
                      glfw glad)
set_target_properties(gl_shader_cache_test PROPERTIES FOLDER renderer)
