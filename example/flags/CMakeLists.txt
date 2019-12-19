set(flags_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/gflags/src)

add_executable(flags ${flags_srcs})
target_link_libraries(flags gflags)
set_target_properties(flags PROPERTIES FOLDER Examples)