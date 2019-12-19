cmake_minimum_required(VERSION 3.1)

project(games)

# Allow creating filters for projects in visual studio.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 17 required for pack fold expression in ecs. Can probably figure
# out an impl without but need this for now.
set(CMAKE_CXX_STANDARD 17)

# Output compile_commands.json for autocomplete.
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_TESTS  OFF CACHE BOOL "" FORCE)
set(GFLAGS_BUILD_TESTING OFF CACHE BOOL "" FORCE)

# Third party libraries.
add_subdirectory(third_party/gflags)
add_subdirectory(third_party/glad)
add_subdirectory(third_party/googletest)
add_subdirectory(third_party/glfw)

# Move third party targets into their own folders for Visual Studio
set_target_properties(gflags_nothreads_static PROPERTIES FOLDER ThirdParty/gflags)
set_target_properties(glad PROPERTIES FOLDER ThirdParty/glad)
set_target_properties(glad-generate-files PROPERTIES FOLDER ThirdParty/glad)

# Our stuff.
add_subdirectory(math)
add_subdirectory(search)
add_subdirectory(network)
add_subdirectory(platform)
add_subdirectory(ecs)
add_subdirectory(renderer)
add_subdirectory(tile)
add_subdirectory(game)
add_subdirectory(integration)
add_subdirectory(example)
add_subdirectory(demo)
