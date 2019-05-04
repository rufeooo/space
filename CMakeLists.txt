cmake_minimum_required(VERSION 3.1)

project(games)

# 17 required for pack fold expression in ecs. Can probably figure
# out an impl without but need this for now.
set(CMAKE_CXX_STANDARD 17)

# Output compile_commands.json for autocomplete.
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Third party libraries.
add_subdirectory(third_party/Catch2)
add_subdirectory(third_party/flatbuffers)
add_subdirectory(third_party/SDL-mirror)

# Our stuff.
add_subdirectory(types)
add_subdirectory(ecs)
add_subdirectory(tile)
add_subdirectory(examples)
