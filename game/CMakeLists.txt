set(game_test_srcs
    "game_test.cc")

set(event_buffer_test_srcs
    "event_buffer_test.cc")

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)

add_library(game_lib "game.cc")
target_link_libraries(game_lib filesystem)

add_executable(game_test ${game_test_srcs})
target_link_libraries(game_test game_lib gtest)
set_target_properties(game_test PROPERTIES FOLDER Game)

add_executable(event_buffer_test ${event_buffer_test_srcs})
target_link_libraries(event_buffer_test game_lib gtest)
set_target_properties(event_buffer_test PROPERTIES FOLDER Game)
