# Interview notes

## ELF and compilation

ELF is a common Unix object-file and executable format. A source file is compiled as a translation unit into an object file; a linker combines object files and libraries into an executable or shared object. A relocatable object usually has no final process addresses.

Sections organize object-file content and metadata (`.text`, `.data`, `.bss`, symbol tables, relocation tables). Segments describe loadable regions in executable/shared-object program headers. LinkMap focuses on sections in `ET_REL` inputs.

## Symbols and relocations

A symbol table stores records; a string table stores the symbol names referenced by offsets. Local binding confines a symbol to its object. Global symbols participate in cross-object lookup. Weak definitions generally have lower precedence than strong definitions, subject to linker rules. An undefined symbol in an object is often a normal external reference.

A relocation records a site, symbol index, relocation type, and (for RELA) explicit addend. In a relocatable object, the offset is interpreted relative to the target section. Relocation is the process of adjusting a location; symbol resolution chooses a definition. A linker must perform both along with layout and many other rules.

Static archive extraction and library search order matter because linkers commonly extract archive members on demand as unresolved symbols appear. LinkMap does not model this or perform a final link.

## Parsing and validation

Offsets and counts are untrusted. Checked arithmetic and range checks prevent a claimed file range from wrapping or escaping the byte buffer. Strings must terminate inside their linked table. Tests combine small synthetic malformed files with real compiler-produced relocatable objects, then compare selected facts against independent tools.

LinkMap is an inspection and diagnostic tool, not a production linker. It never applies relocations, invents final addresses, or guarantees that an unresolved supplied-set reference will fail in a complete build.
