#!/bin/bash
# Script that runs all tests.

source run_constants.sh

SOURCE_DIR='src/'
REPO_TESTS=(`find $SOURCE_DIR -type f -iname "*_test.cc"`)

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

for test_file in ${REPO_TESTS[@]}; do
  ./run_test.sh $test_file
done

