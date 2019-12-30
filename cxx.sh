#!/bin/bash
source ./run_constants.sh

if [ -n "$unix" ]; then
  # clang++ is 10x slower when pinned to core 0, so use taskset to force core 1
  CMD="taskset 0x2 $CXX $CXXFLAG $1 -I src/ -lX11 -lEGL -lGL -lpthread -o $BIN_DIR/`basename -s .cc $1`"
  if [ -n `which perf` ]; then
    perf stat $CMD
  else
    $CMD
  fi
else
  $CXX $CXXFLAG $1 -ObjC++ -I src/ -L bin/ -ldl -o $BIN_DIR/`basename -s .cc $1` -framework OpenGL -framework AppKit -mmacosx-version-min=10.7 -stdlib=libc++
  # For teeny weeny builds '-Os -flto'
fi

# If a second arg exists run the binary
if [ ! -z "$2" ]; then
  ./$BIN_DIR/`basename -s .cc $1`
fi
