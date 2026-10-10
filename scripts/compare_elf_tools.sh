#!/bin/sh
set -eu
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
linkmap_bin=${1:-"$repo/build/linkmap"}
python=${PYTHON:-python3}
compiler=${CC:-clang}
readelf=${READELF:-llvm-readelf}
exec "$python" "$repo/scripts/compare_elf_tools.py" "$linkmap_bin" "$compiler" "$readelf" "$repo/tests/fixtures/source"
