set(asteroids_srcs
    "main.cc")

include_directories(${CMAKE_SOURCE_DIR}/math)

add_executable(asteroids ${asteroids_srcs})
