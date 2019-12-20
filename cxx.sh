#!/bin/bash
clang++ -std=c++17 $1 -I src/ -L src/glfw/ -ldl -lglfw
