#!/bin/bash
# Removes X11/EGL dependencies via linker optimization
CXXFLAGS="-O1 -std=c++17 -flto" ./cxx.sh src/space_server.cc
docker build . -t space
