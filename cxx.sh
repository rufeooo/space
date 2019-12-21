#!/bin/bash
source ./run_constants.sh

if [[ $unix ]]; then
  $CXX -std=c++17 $1 -I src/ -L bin/ -ldl -lglfw -o $BIN_DIR/`basename -s .cc $1`
else
  echo 'test'
fi
