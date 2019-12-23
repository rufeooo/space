#!/bin/bash
source ./run_constants.sh

if [ -n "$unix" ]; then
  $CXX $CXXFLAG $1 -I src/ -L bin/ -lX11 -lEGL -lGL -lpthread -o $BIN_DIR/`basename -s .cc $1`
else
  $CXX $CXXFLAG $1 -ObjC++ -I src/ -L bin/ -ldl -o $BIN_DIR/`basename -s .cc $1` -framework OpenGL -framework AppKit -mmacosx-version-min=10.7 -stdlib=libc++
fi

# If a second arg exists run the binary
if [ ! -z "$2" ]; then
  ./$BIN_DIR/`basename -s .cc $1`
fi
