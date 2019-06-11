BUILD_DIR="build"
LOG_DIR="logs"

# Not a great situation but this assumes logs will go into the root
# of the project and binaries will be run from the same location.
# TODO: Resolve paths for logs - the running c++ binary needs to know
# where the logs should go and that should correspond with whatever
# is here in the batch script. The logging library we are using, plog,
# cannot create dirs itself so maybe the solution here is to modify it
# such that it does.
mkdir $LOG_DIR

# Make the build directory if it doesn't already exist.
if [[ ! -e $BUILD_DIR ]]; then
  mkdir $BUILD_DIR
  cd $BUILD_DIR
  cmake ../
else
  # This assumes if the build directory already exists cmake was run.
  cd $BUILD_DIR
fi
