set(udp_client_srcs
    "network_udp_client.cc")

build_flatbuffers(
    "protocol/asteroids_packet.fbs" "" fake_asteroids_packet_fbs ""
    "protocol/" "" "")


include_directories(${CMAKE_SOURCE_DIR}/third_party/gflags/src)
include_directories(${CMAKE_SOURCE_DIR})

add_executable(network_udp_client ${udp_client_srcs})
target_link_libraries(network_udp_client network gflags flatbuffers 
                      client)

add_dependencies(network_udp_client fake_asteroids_packet_fbs)
