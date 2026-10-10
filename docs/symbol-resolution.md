# Supplied-object symbol resolution

LinkMap builds an index from named eligible global/weak definitions in the input object set and compares relocation references against it. Local symbols are never made cross-object providers. Default and protected visibility are considered; hidden/internal visibility is outside this model. Strong global duplicates are reported, and weak-only matches receive a note.

An unresolved result means only “unresolved within supplied object set.” A separate archive, shared library, startup object, linker script, or dynamic symbol source may satisfy the reference. Weak undefined symbols may legally remain unresolved and are not reported as ordinary missing strong references. Common-symbol coalescing, versions, COMDAT, archive extraction, and complete linker precedence are omitted.

Resolution explains selected relationships. It is not a linker's final decision.
