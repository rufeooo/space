set(logging_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/plog/include)
add_executable(logging ${logging_srcs})
target_link_libraries(logging plog)
