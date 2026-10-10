# Contributing

Keep parser changes byte-oriented and bounds-checked. Add compiler-generated ELF regression coverage for semantic changes and synthetic bytes for malformed cases. Preserve deterministic output and stable diagnostic IDs. Do not claim a linker rule unless it is implemented and tested.

Build and validate with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Run `scripts/run_sanitizers.sh` when a compatible Clang toolchain is available. Keep fixtures in temporary build/test directories; tests must not rewrite checked-in inputs.
