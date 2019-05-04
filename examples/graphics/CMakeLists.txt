set(graphics_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/SDL-mirror/include)

add_executable(graphics ${graphics_srcs})
target_link_libraries(graphics SDL2)
