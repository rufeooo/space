# Script that runs all tests.

source run_constants.sh

TARGET_DIR='src/'
REPO_TESTS=(`find $TARGET_DIR -type f -iname "*_test.cc"`)

if [ -z "$CXX" ]; then
	CXX=`which clang++`
fi
if [ -z "$CXX" ]; then
	CXX=`which g++`
fi
if [ -z "$CXX" ]; then
	echo "Failed to detect compiler"
	exit 1
fi

GTEST_DIR='third_party/googletest/googletest/'
# Manually make all test targets and run them.
# TODO: Is there a more clever way to find all test targets? Maybe
# cmake can intelligently make a target for all test files.
for test_file in ${REPO_TESTS[@]}; do
  echo $CXX $test_file
  $CXX -std=c++17 -I $TARGET_DIR -I $GTEST_DIR -I $GTEST_DIR/include $test_file $GTEST_DIR/src/gtest-all.cc -lpthread
done

