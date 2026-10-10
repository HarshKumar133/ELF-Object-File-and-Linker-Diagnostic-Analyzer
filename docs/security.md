# Security

ELF files are untrusted input. LinkMap reads object bytes and does not execute or modify them. Input size is capped at 512 MiB per file. Every section, table, string, and symbol/relocation index is checked before use. Decoding is byte-oriented and avoids native structure casts, unaligned access, and host-endian assumptions.

The CLI passes paths directly to file APIs and does not build shell commands. External comparison scripts use quoted argument arrays. Output creation refuses an existing path rather than overwriting it. Filesystem and parse failures produce a diagnostic instead of an intentional crash.

The cap and fixed ELF entry sizes limit file-controlled allocations. Multi-object total memory is not globally capped, so callers should still use reasonable input sets.
