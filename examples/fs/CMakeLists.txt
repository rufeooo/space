set(fs_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/cppfs/source/include)

add_executable(fs ${fs_srcs})
target_link_libraries(fs cppfs)
set_target_properties(fs PROPERTIES FOLDER Examples)