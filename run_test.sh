#!/bin/bash
# Script that runs a single test

if [ -z "$1" ]; then
  echo "Usage: ./run_test.sh <*_test.cc>"
fi

if [ -z $BUILD_DIR ]; then
	source ./run_constants.sh
fi

if [ -z "$SOURCE_DIR" ]; then
	SOURCE_DIR=src/
fi

GTEST_DIR='third_party/googletest/googletest/'
echo $CXX $1
$CXX -std=c++17 -I $SOURCE_DIR -I $GTEST_DIR -I $GTEST_DIR/include $1 -include $GTEST_DIR/src/gtest-all.cc -lpthread
