# ELF overview and supported subset

ELF (Executable and Linkable Format) describes object files, executables, shared objects, and core files. LinkMap currently accepts ELF64 little-endian relocatable files (`ET_REL`). It reports the file type, machine number/name, class, byte order, ABI identification, entry value, and table locations.

The generic ELF header/table parser can read 64-bit little-endian `ET_REL` inputs for machines other than x86-64. Only selected x86-64 relocation types receive descriptions. This does not imply that machine-specific symbol or relocation semantics are understood for every machine.

ELF32, big-endian data, `ET_EXEC`, `ET_DYN`, extended section numbering, and `SHN_XINDEX` symbol indexes are currently unsupported. Extended counts are rejected explicitly rather than interpreted as ordinary values. The parser never treats ELF32 fields as ELF64 fields.

The host used for validation was macOS arm64, but compiler-generated test inputs were x86-64 Linux ELF objects emitted by Clang's target mode. This is evidence for that subset, not universal compatibility.
