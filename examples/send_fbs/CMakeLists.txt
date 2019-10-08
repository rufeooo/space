set(send_fbs_sources
    "main.cc")

set(packet_fbs_sources
    "fbs/packet.fbs")

build_flatbuffers(
    "${packet_fbs_sources}" "" packet_fbs "" "fbs/" "" "")

include_directories(${CMAKE_SOURCE_DIR})

add_executable(send_fbs ${send_fbs_sources})
target_link_libraries(send_fbs flatbuffers client server)
add_dependencies(send_fbs packet_fbs)
