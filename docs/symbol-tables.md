# Symbol tables

`SHT_SYMTAB` and `SHT_DYNSYM` entries are decoded as ELF64 records. The symbol table's `sh_link` points to its string table. Names are bounded by that table. Each record retains its table section and table-local index, name, value, size, binding, type, visibility, and section index.

Undefined means `st_shndx == SHN_UNDEF`; a zero value by itself is not used. `SHN_ABS` and `SHN_COMMON` are identified. Local symbols do not enter cross-object resolution. Candidate global/weak definitions are limited to named NOTYPE, OBJECT, FUNC, TLS, and IFUNC records with default/protected visibility. Hidden/internal and common symbols are outside the implemented matching model.

Extended `SHN_XINDEX` resolution via `SHT_SYMTAB_SHNDX` is not implemented. Symbol versions and GNU-specific symbol semantics are not interpreted.
