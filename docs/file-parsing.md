# File parsing

All input is read as bytes and decoded explicitly. `CheckedReader` checks `offset <= size` and `length <= size - offset`, avoiding overflowing `offset + length` checks. Integer reads are limited to widths 1 through 8 and never use native struct casts or alignment assumptions. Strings must terminate before the linked string-table end.

The current file reader caps each object at 512 MiB. ELF64 fixed-width table entries are validated before iteration; section payload ranges, linked indexes, symbol indexes, and table entry divisibility are checked before use. Parser errors carry a stable code, message, and input offset.

The reader currently supports ELF little-endian only. It has byte-order parameterization for integer reads, but the ELF parser rejects big-endian files instead of implying that the complete format is supported.
