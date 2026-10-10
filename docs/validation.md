# Validation

CTest runs the custom C++ unit harness and a Python integration test. The integration test compiles checked-in C fixtures to ELF64 x86-64 objects with Clang's Linux target and checks text, JSON, DOT, matched/unmatched symbols, duplicate strong definitions, malformed paths, and exit codes. It compiles into a temporary directory and does not link or execute generated object files.

`scripts/compare_elf_tools.sh` compares selected facts with LLVM `llvm-readelf`, `llvm-nm`, and `llvm-objdump` when available. Tool versions and skipped tools should be recorded with the actual validation result. Do not compare full human-formatted output across tool versions.

Sanitizers can be attempted with `scripts/run_sanitizers.sh`. This repository has no performance claims; `benchmarks/benchmark.py` reports measured work only when explicitly run.
