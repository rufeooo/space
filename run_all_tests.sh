# Script that runs all tests.

source run_constants.sh

# Manually make all test targets and run them.
# TODO: Is there a more clever way to find all test targets? Maybe
# cmake can intelligently make a target for all test files.
make ecs_test
./ecs/ecs_test

make system_test
./ecs/system_test

make search_test
./search/search_test

make vec_test
./math/vec_test

make mat_test
./math/mat_test

make quat_test
./math/quat_test

make mat_ops_test
./math/mat_ops_test

make server_test
./network/server_test

make intersection_test
./math/intersection_test

make game_test
./game/game_test

make observer_test
./observer/observer_test

make gl_shader_cache_test
./renderer/gl_shader_cache_test
