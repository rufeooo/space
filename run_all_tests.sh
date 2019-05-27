# Script that runs all tests.

source run_constants.sh

# Manually make all test targets and run them.
# TODO: Is there a more clever way to find all test targets? Maybe
# cmake can intelligently make a target for all test files.
make ecs_test
./ecs/ecs_test

make search_test
./search/search_test