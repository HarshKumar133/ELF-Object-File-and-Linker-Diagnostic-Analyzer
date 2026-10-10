# Diagnostics and exit codes

| ID | Meaning |
|---|---|
| `LM1001` | Non-ELF magic or truncated identification |
| `LM1002` | Unsupported ELF class |
| `LM1003` | Unsupported byte order |
| `LM1005`–`LM1014` | Header, table, unsupported feature, or input-file failure |
| `LM2001`–`LM2004` | Section table, payload, or name-table failure |
| `LM3001`–`LM3007` | Symbol/string table or symbol-index failure |
| `LM4001`–`LM4007` | Relocation layout, links, or symbol-index failure |
| `LM5001` | Reference unresolved within supplied set |
| `LM5002` | Multiple strong definitions in supplied set |
| `LM5003` | Weak-only definition match |
| `LM5004` | Duplicate weak definitions; linker-specific selection |

CLI exit codes are 0 for no findings, 1 when analysis has findings, 2 for CLI errors, 3 for malformed/read failures, 4 for unsupported formats, and 7 for output errors. A warning is not a claim of a guaranteed final link failure. JSON stdout contains only JSON; operational failures go to stderr.

Finding records retain severity, category, symbol, source object, relocation site when available, message, action, related object IDs where relevant, and a `supported-subset` confidence label. Source line numbers are not invented.
