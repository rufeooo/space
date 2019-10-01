set(socket_srcs
    "main.cc")

add_executable(socket ${socket_srcs})
set_target_properties(socket PROPERTIES FOLDER Examples)
