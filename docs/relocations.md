# Relocations

For each `SHT_REL` or `SHT_RELA` section, `sh_link` selects its symbol table and `sh_info` selects its target section. The parser checks these relationships and every symbol index. ELF64 REL entries are 16 bytes; RELA entries are 24 bytes. The `r_info` upper 32 bits are the symbol-table index and the lower 32 bits are the raw type.

For `ET_REL`, `r_offset` normally identifies a location in the target section, not a process virtual address. RELA's signed addend is reported; REL has no explicit addend field. Selected x86-64 types (including 64, PC32, PLT32, GOTPCREL, 32/32S, GOTPCRELX and REX_GOTPCRELX) have names and short descriptions. Unknown values remain numeric and are not guessed.

LinkMap does not evaluate relocation formulas, patch sections, or produce output code. Named relocation descriptions are not proof that a relocation application is supported.
