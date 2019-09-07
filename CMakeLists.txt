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
add_subdirectory(third_party/Catch2)
add_subdirectory(third_party/cppfs)
add_subdirectory(third_party/flatbuffers)
add_subdirectory(third_party/gflags)
add_subdirectory(third_party/glad)
add_subdirectory(third_party/plog)
add_subdirectory(third_party/glfw)
add_subdirectory(third_party/SDL-mirror)

# Move third party targets into their own folders for Visual Studio
set_target_properties(flatbuffers PROPERTIES FOLDER ThirdParty/flatbuffers)
set_target_properties(flatc PROPERTIES FOLDER ThirdParty/flatbuffers)
set_target_properties(flathash PROPERTIES FOLDER ThirdParty/flatbuffers)
set_target_properties(gflags_nothreads_static PROPERTIES FOLDER ThirdParty/gflags)
set_target_properties(glad PROPERTIES FOLDER ThirdParty/glad)
set_target_properties(glad-generate-files PROPERTIES FOLDER ThirdParty/glad)
set_target_properties(pack PROPERTIES FOLDER ThirdParty/cppfs)
set_target_properties(pack-cppfs PROPERTIES FOLDER ThirdParty/cppfs)
set_target_properties(SDL2 PROPERTIES FOLDER ThirdParty/SDL)
set_target_properties(SDL2main PROPERTIES FOLDER ThirdParty/SDL)
set_target_properties(SDL2-static PROPERTIES FOLDER ThirdParty/SDL)
set_target_properties(check-cppfs PROPERTIES FOLDER ThirdParty/cppfs)
set_target_properties(component_install PROPERTIES FOLDER ThirdParty/cppfs)
set_target_properties(check-all PROPERTIES FOLDER ThirdParty/cppfs)
set_target_properties(cppfs PROPERTIES FOLDER ThirdParty/cppfs)
set_target_properties(check-template PROPERTIES FOLDER CMakePredefinedTargets)
set_target_properties(test PROPERTIES FOLDER ThirdParty/cppfs)

# Our stuff.
add_subdirectory(types)
add_subdirectory(math)
add_subdirectory(search)
add_subdirectory(ecs)
add_subdirectory(tile)
add_subdirectory(game)
add_subdirectory(examples)
add_subdirectory(demos)
