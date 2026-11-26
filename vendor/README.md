# SDK interfaces

The headers in `include/` **are used by game code**. They provide guarded
SDK includes, missing API declarations and case-sensitive include shims for
the pinned Runtime Library 3.3 headers. Their public include names remain
`psyq/*.h`, `sys/*.h` and the lower-case shims (`asm.h`, `r3000.h`); the
compiler searches `vendor/include/` before the separately supplied SDK
headers.

The shims were written against the Psy-Q 3.0 headers of the King's Field II
project, and their comments still name that release. They compile against
the 3.3 headers (`tests/test_sdk_headers.py`), but each comment's claim about
what a header omits is not yet re-checked against 3.3.

The executable build links the original Runtime Library 3.3 objects and
libraries. No Sony library routines are reconstructed here; SDK functions are
identified in `config/retail/functions_vendored.tsv` and excluded from game
progress. Original project licensing does not grant rights to Sony/Psy-Q
material.
