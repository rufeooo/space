#!/bin/bash
source ./run_constants.sh

if [ -n "$unix" ]; then
  $CXX $CXXFLAG $1 -I src/ -L bin/ -lX11 -lEGL -lGL -lpthread -o $BIN_DIR/`basename -s .cc $1`
else
  $CXX $CXXFLAG $1 -ObjC++ -I src/ -L bin/ -ldl -o $BIN_DIR/`basename -s .cc $1` -framework OpenGL -framework AppKit -mmacosx-version-min=10.7 -stdlib=libc++
fi
