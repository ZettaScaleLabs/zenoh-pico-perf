#!/bin/false

root_dir=$(git rev-parse --show-toplevel)
lib_dir=$root_dir/install
export CPATH="$lib_dir/include:$CPATH"
export LIBRARY_PATH="$lib_dir/lib:$LIBRARY_PATH"
export LD_LIBRARY_PATH="$lib_dir/lib:$LD_LIBRARY_PATH"
