BUILD_DIR="build"

# Make the build directory if it doesn't already exist.
if [[ ! -e $BUILD_DIR ]]; then
  mkdir $BUILD_DIR
  cd $BUILD_DIR
  cmake ../
else
  # This assumes if the build directory already exists cmake was run.
  cd $BUILD_DIR
fi
