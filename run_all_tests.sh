# Script that runs all tests.

source run_constants.sh

# Manually make all test targets and run them.
# TODO: Is there a more clever way to find all test targets? Maybe
# cmake can intelligently make a target for all test files.
make ecs_test
./src/ecs_test

make vec_test
./src/vec_test

make mat_test
./src/mat_test

make quat_test
./src/quat_test

make mat_ops_test
./src/mat_ops_test

#make server_test
#./src/server_test

#make client_test
#./src/client_test

make intersection_test
./src/intersection_test

make game_test
./src/game_test

make event_buffer_test
./src/event_buffer_test

#make gl_shader_cache_test
#./src/gl_shader_cache_test

#make search_test
#./src/search_test

