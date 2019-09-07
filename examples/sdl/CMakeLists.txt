set(sdl_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/SDL-mirror/include)

add_executable(sdl_test ${sdl_srcs})
target_link_libraries(sdl_test SDL2)
set_target_properties(sdl_test PROPERTIES FOLDER Examples)