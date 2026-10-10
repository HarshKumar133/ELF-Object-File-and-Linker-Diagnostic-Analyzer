# Sections

Section headers describe object-file content and metadata. LinkMap reports index, name, type, flags, address, file offset, size, link, info, alignment, and entry size. Section names are read only from the declared section-name string table and every name is bounded by that table.

`SHT_NOBITS` sections such as `.bss` represent memory size without corresponding file bytes. Their ranges are not interpreted as file payload. All other non-null section payloads must fit in the input. Unknown section types are retained and labeled `OTHER`; their payload is not interpreted beyond range checking.

The tool shows raw ELF type/flags rather than inferring a complete load layout. Sections are not ELF segments.
