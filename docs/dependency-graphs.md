# Dependency graphs

The object graph has stable IDs `object_N` assigned by input order. Each edge records an actual relocation symbol reference matched to an eligible definition in another supplied object; labels include the symbol and section-relative offset. Repeated reference sites remain separate edges. Unresolved relocations point to explicit unresolved diamond nodes in the DOT output.

`linkmap graph objects...` emits the object dependency view. `linkmap graph objects... --symbol NAME` emits a symbol relationship view with definition and relocation-reference links. Neither view represents archive extraction or complete link order. JSON includes dependency source, target, symbol, relocation section, and offset.
