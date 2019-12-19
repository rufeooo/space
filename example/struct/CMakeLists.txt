set(struct_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR})

add_executable(struct ${struct_srcs})
set_target_properties(struct PROPERTIES FOLDER Examples)
