include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)

include_directories(${CMAKE_SOURCE_DIR}/third_party/glad)
include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/include)

add_executable(space "space.cc" )

target_link_libraries(space glfw glad)
