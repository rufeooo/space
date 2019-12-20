#!/bin/bash
source ./run_constants.sh

$CC src/glfw/glfw.c -D_GLFW_X11 -I third_party/glfw/src/ -ldl -lX11 -lpthread -lm -fpic -shared -o $BIN_DIR/libglfw.so
