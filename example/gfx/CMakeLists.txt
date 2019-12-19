set(gl_version_srcs
    "gl_version.cc")

set(gl_colored_triangle_srcs
    "gl_colored_triangle.cc")

set(gl_triangle_srcs
    "gl_triangle.cc")

set(gl_triangle_translate_srcs
    "gl_triangle_translate.cc")

function(create_gfx_example name srcs)
  include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/glfw/include)
  add_executable(${name} ${${srcs}})
  target_link_libraries(${name} glfw glad)
  set_target_properties(${name} PROPERTIES FOLDER Examples/gfx)
endfunction(create_gfx_example)

create_gfx_example(gl_version gl_version_srcs)
create_gfx_example(gl_colored_triangle gl_colored_triangle_srcs)
create_gfx_example(gl_triangle gl_triangle_srcs)
create_gfx_example(gl_triangle_translate gl_triangle_translate_srcs)
