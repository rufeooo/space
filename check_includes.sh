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

