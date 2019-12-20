#!/bin/bash
source ./run_constants.sh

$CXX -std=c++17 $1 -I src/ -L bin/ -ldl -lglfw -o $BIN_DIR/`basename -s .cc $1`
