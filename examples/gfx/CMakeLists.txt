set(gl_version_srcs
    "gl_version.cc")

set(gl_triangle_srcs
    "gl_triangle.cc")

function(create_gfx_example name srcs)
  include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/include)
  add_executable(${name} ${${srcs}})
  target_link_libraries(${name} glfw glad)
endfunction(create_gfx_example)

create_gfx_example(gl_version gl_version_srcs)
create_gfx_example(gl_triangle gl_triangle_srcs)
