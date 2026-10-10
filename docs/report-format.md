# Report format

JSON is UTF-8, deterministic for a fixed input order, and uses `schema_version: 1`. Top-level keys are:

- `schema_version`, `tool_version`, `command`, `analysis_complete`
- `input_files`: stable input ID/path, ELF class, byte order, type/machine numbers and names, ABI/version fields, entry point, program/section table offsets and counts, plus arrays of sections, symbols, and relocations
- `object_dependencies`: source and target object IDs, symbol, relocation section, and section-relative offset
- `unresolved_references`, `duplicate_definitions`, and `diagnostics`

Section records include index, name, type/name, flags, address, file offset, size, link/info, alignment, entry size, and whether file-backed. Symbol records include symbol-table section/index, name, value/size, binding, type, visibility, section index, and undefined status. Relocation records include relocation and target section, offset, raw type/type name, symbol-table index/name, addend state/value, and whether LinkMap has a description.

All strings are escaped by the serializer. Collections retain section/table order or are sorted by stable diagnostic/dependency keys. JSON stdout contains no operational logging. The report does not include resolved final addresses or imply a complete link.
