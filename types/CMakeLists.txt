set(types_sources
    "main.cc")

set(types_common_sources
    "common/vec3.fbs")

build_flatbuffers(
  "${types_common_sources}" "" types_common "" "common/" "" "")

include_directories(./)

add_executable(types ${types_sources})
target_link_libraries(types flatbuffers)
add_dependencies(types types_common)
set_target_properties(types PROPERTIES FOLDER Types)
set_target_properties(types_common PROPERTIES FOLDER Types)
