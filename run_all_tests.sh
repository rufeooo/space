# Script that runs all tests.

source run_constants.sh

REPO_TESTS=(`find src -type f -iname "*_test.cc"`)

pushd $BUILD_DIR
# Manually make all test targets and run them.
# TODO: Is there a more clever way to find all test targets? Maybe
# cmake can intelligently make a target for all test files.
for test in ${REPO_TESTS[@]}; do
  echo make $test
  make `basename -s .cc $test`
  # or maybe this?
  # clang++ -std=c++17 -I third_party/googletest/googletest/include/ src/math/mat_ops_test.cc -L build/lib/ -lgtest -lpthread
done
popd
