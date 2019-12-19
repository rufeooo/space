include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)

include_directories(${CMAKE_SOURCE_DIR}/third_party/glad)
include_directories(${CMAKE_SOURCE_DIR}/third_party/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/gflags)

add_executable(space "space.cc" )

target_link_libraries(space glfw glad)
add_executable(asteroids_client "asteroids_client.cc")
target_link_libraries(asteroids_client glfw glad gflags)
add_executable(asteroids_server "asteroids_server.cc")
target_link_libraries(asteroids_server glfw glad gflags)

function(create_test files target_name)
  include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
  include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)
  add_executable(${target_name} ${files})
  target_link_libraries(${target_name} gtest gmock)
endfunction()

create_test("ecs/ecs_test.cc" ecs_test)
create_test("game/event_buffer_test.cc" event_buffer_test)
create_test("game/game_test.cc" game_test)
create_test("math/intersection_test.cc" intersection_test)
create_test("math/mat_ops_test.cc" mat_ops_test)
create_test("math/mat_test.cc" mat_test)
create_test("math/quat_test.cc" quat_test)
create_test("math/vec_test.cc" vec_test)
