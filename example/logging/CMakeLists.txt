set(logging_srcs
    "main.cc")

add_executable(logging ${logging_srcs})
set_target_properties(logging PROPERTIES FOLDER Examples)
