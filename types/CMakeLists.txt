set(types_sources
    "main.cc")

# Generate flatbuffers libs.
include(${CMAKE_SOURCE_DIR}/third_party/flatbuffers/CMake/BuildFlatBuffers.cmake)

set(types_common_sources
    "common/vec3.fbs")

build_flatbuffers(
  "${types_common_sources}" "" types_common "" "common/" "" "")

include_directories(./)

add_executable(types ${types_sources})
target_link_libraries(types flatbuffers)
add_dependencies(types types_common)
