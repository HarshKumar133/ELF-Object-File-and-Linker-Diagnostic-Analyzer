#!/bin/sh
set -eu
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=${1:-"$repo/build/tests"}
compiler=${CXX:-clang++}
cmake -S "$repo" -B "$build_dir" -DCMAKE_CXX_COMPILER="$compiler" -DCMAKE_BUILD_TYPE=Debug -DLINKMAP_BUILD_TESTS=ON
cmake --build "$build_dir" --parallel
ctest --test-dir "$build_dir" --output-on-failure
