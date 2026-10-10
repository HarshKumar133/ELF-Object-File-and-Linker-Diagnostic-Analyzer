# Architecture

`CheckedReader` performs little-endian fixed-width reads, checked ranges, and bounded string extraction. `parseHeader` validates identification and ELF64 `ET_REL` header fields. `parseSections`, `parseSymbols`, and `parseRelocations` consume normalized metadata and return structured `Result` errors. `parseObject` composes these stages. `analyze` indexes eligible supplied-set definitions, diagnoses selected issues, and builds object dependency edges. `report.cpp` serializes normalized records; `main.cpp` handles argument parsing, files, and exit codes.

The core library has no terminal or CLI dependencies. Input symbol identity is `(object id, symbol-table section, symbol index)`; table-local indexes are never global identities. Collections and diagnostics are sorted before reporting.

## Current analysis boundaries

The definition index includes named global/weak symbols with default or protected visibility, excluding common symbols. Supported candidate types are NOTYPE, OBJECT, FUNC, TLS, and GNU IFUNC. Strong duplicate checks count multiple global definitions. A relocation to an undefined global/weak symbol is unresolved only against the supplied object set; weak undefined references are allowed to remain unmatched. A matching weak definition gets a note when no strong definition exists.

No relocation is applied. Dependency edges show parsed relocation references matched to definitions in supplied objects; they are not link order or runtime dependencies.
