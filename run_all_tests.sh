#!/bin/bash
# Script that runs all tests.

source run_constants.sh

SOURCE_DIR='src/'
REPO_TESTS=(`find $SOURCE_DIR -type f -iname "*_test.cc"`)

for test_file in ${REPO_TESTS[@]}; do
  ./run_test.sh $test_file
done

