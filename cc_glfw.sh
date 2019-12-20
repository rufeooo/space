#!/bin/bash
clang src/glfw/glfw.c -D_GLFW_X11 -I third_party/glfw/src/ -ldl -lX11 -lpthread -lm -fpic -shared -o libglfw.so
