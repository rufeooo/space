#!/bin/bash
# CXXFLAG override removes unnecessary X11/EGL dependencies
CXXFLAGS="-O1 -std=c++17 -flto -Wl,-as-needed" ./cxx.sh src/space_server.cc
docker build . -t space
