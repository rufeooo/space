# A helper script so you can build targets from the current
# directory.
#
# Usage:
# ./run_make.sh <target_name>
#
# To build all targets
# ./run_make.sh

source run_constants.sh

make $1 -j 16
