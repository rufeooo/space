#!/bin/bash
BUILD_DIR="build"
LOG_DIR="logs"
BIN_DIR="bin"

# Not a great situation but this assumes logs will go into the root
# of the project and binaries will be run from the same location.
# TODO: Resolve paths for logs - the running c++ binary needs to know
# where the logs should go and that should correspond with whatever
# is here in the batch script. The logging library we are using, plog,
# cannot create dirs itself so maybe the solution here is to modify it
# such that it does.
mkdir $LOG_DIR -p

# Make the build directory if it doesn't already exist.
mkdir $BUILD_DIR -p
# Make the bin directory if it doesn't already exist.
mkdir $BIN_DIR -p

# detect compiler for c++
if [ -z "$CXX" ]; then
	CXX=`which clang++ || which g++`
fi
if [ -z "$CXX" ]; then
	echo "Failed to detect C++ compiler"
	exit 1
fi

# detect compiler for C
if [ -z "$CC" ]; then
	CC=`which clang || which gcc`
fi
if [ -z "$CC" ]; then
	echo "Failed to detect C compiler"
	exit 1
fi
