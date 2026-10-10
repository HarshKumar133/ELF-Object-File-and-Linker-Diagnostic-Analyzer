# Limitations and future work

Current support is ELF64 little-endian `ET_REL`. ELF32, big-endian, executables, shared objects, extended numbering, `SHN_XINDEX`, archives, and dynamic symbol resolution are not implemented. The parser reads generic 64-bit little-endian table layouts; only a selected x86-64 relocation subset has descriptions.

The resolver is an explanatory supplied-object model. It does not implement symbol versions, linker scripts, COMDAT/group selection, archive search/extraction, all common-symbol rules, hidden-symbol diagnostics, dynamic lookup, TLS relocation semantics, IFUNC execution, or exact GNU ld/LLD precedence. Relocations are reported but never applied. There is no DWARF decoding or final address layout.

High-value next steps are: (1) property/fuzz tests plus complete ASan/UBSan runs on malformed data; (2) extended section numbering and symbol-index support; (3) deeper ABI-derived x86-64 relocation/reference comparisons and resolver cases such as common symbols and visibility.
