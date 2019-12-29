#!/bin/bash
source ./run_constants.sh

ARR=$(find src -type f -iname "*.h")

for file in ${ARR[@]}; do
  echo "int main() { return 0; }" | $CXX $CXXFLAG -x c++ -c -include $file -I src/ -
  if [ $? -ne 0 ]; then
	  echo $file failed
	  exit 1
  fi
  echo $file checked
done

ARR=$(find src -type f -iname "*.cc")
GTEST_DIR='third_party/googletest/googletest/'
GTEST_MAIN=`cat $GTEST_DIR/src/gtest-all.cc`
for file in ${ARR[@]}; do
  file_basename=`basename $file`
  if [ "$file_basename" != ${file_basename#win32} ]; then
	  echo SKIP $file_basename [platform file]
	  continue
  elif [ "$file_basename" != ${file_basename#unix} ]; then
	  echo SKIP $file_basename [platform file]
	  continue
  fi
  echo $GTEST_MAIN | $CXX $CXXFLAG -x c++ -c -include $file -I src/ -I $GTEST_DIR -I $GTEST_DIR/include -
  if [ $? -ne 0 ]; then
	  echo $file failed
	  exit 1
  fi
  echo $file checked
done
