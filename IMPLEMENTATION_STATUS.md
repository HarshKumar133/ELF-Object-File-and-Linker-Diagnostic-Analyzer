# LinkMap implementation status

The project is being implemented in the phases specified by `InformationNeeded.txt`.

| Phase | Scope | Status |
|---|---|---|
| 0 | Environment, C++20/CMake skeleton, help/version, CTest | Complete |
| 1 | Checked reader and ELF identification/header | Complete |
| 2 | Section headers and names | Complete |
| 3 | Symbol tables and reports | Complete |
| 4 | Relocations and x86-64 descriptions | Complete |
| 5 | Cross-object symbol analysis and diagnostics | Complete |
| 6 | Dependency graph and DOT output | Complete |
| 7 | Stable reports and complete CLI | Complete |
| 8 | Toolchain comparison | Complete for selected facts against LLVM readelf, nm, and objdump |
| 9 | Malformed-input robustness and sanitizers | Complete for bounded regression corpus and UBSan; ASan unavailable on this host |
| 10 | Documentation and portfolio polish | Complete |

## Environment observations

- Development host: macOS arm64 (Darwin 27).
- Compiler: Apple Clang 21.0.0; CMake 4.1.2.
- Python 3.14.3 and Git 2.54.0 are available.
- `nm`, `objdump`, and `ld` are available; `readelf` was not found on `PATH`.
- Homebrew LLVM 21.1.4 supplies `clang`, `llvm-readelf`, `llvm-nm`, and `llvm-objdump`.
- Clang successfully emitted x86-64 Linux ELF objects with `--target=x86_64-unknown-linux-gnu`.
- The system Apple Clang C++ standard-library headers are incomplete in this environment; project builds used `/opt/homebrew/opt/llvm/bin/clang++`.

## Validation actually run

- Debug configure/build: `cmake -S . -B build/final -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ -DCMAKE_BUILD_TYPE=Debug -DLINKMAP_BUILD_TESTS=ON` and `cmake --build build/final -j2` — passed.
- CTest: `ctest --test-dir build/final --output-on-failure` — 4/4 passed, including compiler-generated Linux ELF integration.
- UBSan configure/build and CTest with `-fsanitize=undefined` — 4/4 passed, no reported undefined behavior.
- External comparison: `READELF=/opt/homebrew/opt/llvm/bin/llvm-readelf NM=/opt/homebrew/opt/llvm/bin/llvm-nm OBJDUMP=/opt/homebrew/opt/llvm/bin/llvm-objdump scripts/compare_elf_tools.sh build/final/linkmap` — passed for machine, `ET_REL`, undefined symbol, and relocation; Clang and LLVM utilities were version 21.1.4.
- ASan+UBSan binaries built but stalled at process startup, including `linkmap version`; the run was interrupted and is not counted as a pass.
- A deterministic 10,000-case short-byte header exercise is included in the unit test. No libFuzzer campaign or benchmark run is claimed.

## Remaining limitations

See `docs/limitations.md`. The key boundaries are ELF64 little-endian `ET_REL`, selected x86-64 relocation descriptions, incomplete production-linker symbol semantics, and lack of ASan validation on this host.
