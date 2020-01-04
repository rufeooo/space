#!/bin/bash
./cxx.sh src/space_server.cc
docker build . -t space
