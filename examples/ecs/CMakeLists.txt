set(ecs_srcs
    "main.cc")

add_executable(ecs ${ecs_srcs})
set_target_properties(ecs PROPERTIES FOLDER Examples)