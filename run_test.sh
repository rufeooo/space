# Script that runs a single test

if [ -z "$1" ]; then
  echo "Usage: ./run_test.sh <*_test.cc>"
fi

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

if [ -z "$SOURCE_DIR" ]; then
	SOURCE_DIR=src/
fi

GTEST_DIR='third_party/googletest/googletest/'
echo $CXX $1
$CXX -std=c++17 -I $SOURCE_DIR -I $GTEST_DIR -I $GTEST_DIR/include $1 $GTEST_DIR/src/gtest-all.cc -lpthread
