# Function, global, and static identities

The structural census says where linked code and data exist. The identity
layer records the current source-level interpretation without changing delink
symbols or pretending that WIP names are original symbols:

- `config/retail/functions.tsv` lists every admitted function start and WIP
  extent, and `data.tsv` accounts for every other loaded byte (defined data
  or an `unclassified` gap);
- `config/retail/functions_vendored.tsv` layers Sony/Psy-Q ownership over the
  structural rows;
- `config/retail/function_identities.tsv` has exactly one row for each
  carveable non-vendored function;
- `config/retail/data_identities.tsv` tracks initialized objects and referenced
  BSS objects that may become C globals, file-local statics, or
  function-local statics;
- `config/retail/structures.tsv` and `structure_fields.tsv` track complete
  target sizes and every field offset/extent, including opaque ranges; and
- address-derived `func_` and `DAT_` names are stable unresolved identities.

## Current census

| Image | Game functions | `proven` | `candidate` | Bytes |
| :---- | -------------: | -------: | ----------: | ----: |
| `PSX.EXE` | 1 | 1 | 0 | `0xe0` |
| `GAME.EXE` | 593 | 521 | 72 | `0x42638` |
| `OPEN.EXE` | 34 | 33 | 1 | `0x26c8` |
| `END.EXE` | 13 | 12 | 1 | `0xa3c` |
| 28 `FDAT_nnn.BIN` | 436 (14–22 each) | 10 | 426 | `0xb124` |
| **Total** | **1,077** | **577** | **500** | |

`functions.tsv` also keeps the four PS-X EXE entry rows (Psy-Q startup,
already in `functions_vendored.tsv`). All game identities are address-only
with candidate signatures from the static MIPS live-in heuristic
(`kf inventory seed`); no data identities are admitted yet.

### Evidence tiers

Each row's note starts with its AGENTS.md tier. `tier=proven` is decoded
direct control flow: the PS-X EXE entry, or the target of a `jal` or a
cross-function `j` decoded from carved code, including the 52 `GAME.EXE`
functions that FDAT modules call directly. `tier=candidate` is everything
else: 412 FDAT rows are named by their module's dispatch table (stored
pointers that `GAME.EXE` calls indirectly), and the rest are carved from
frame or return shape alone. No function is `validated` yet: that tier needs
reviewed relocation rows, and `relocs.tsv` is still empty.

### Function starts

Starts come from `kf-function-audit` and `kf-function-propose`, rerun until
the proposal stops changing (three passes from the entry-only census). Each
row's `confidence` names the rule that admitted it:

- `entry`: the PS-X EXE header pc0;
- `mips-frame-carve`: a stack frame with `ra` saved and a matching stack
  restore at the return. The game code's GCC 2.6/2.7 restores the stack
  before the return (`lw ra; addiu sp; jr ra; nop`); the library band has
  the older delay-slot form (`jr ra; addiu sp`). Loads scheduled above the
  frame allocation start at the previous function's return;
- `mips-return-carve`: valid code between one return and the next;
- `mips-call-carve`: a `jal` target decoded from admitted code that lies in
  unowned bytes, carved to its first unconditional transfer;
- `mips-call-split`: a `jal` target inside an admitted extent whose preceding
  code cannot fall through into it; and
- `mips-handler-carve`: an FDAT dispatch-table pointer, carved the same way
  (the shared `jr ra; nop` default handler directly after each header).

A `jal` target up to 0x40 bytes of straight-line code above an admitted
start moves that start back (the row's note records it). ASPSX expands
every `div` with `break 7`/`break 6` traps whose branches only skip the
trap; such a run counts as straight-line, and each trap widens the window
by one expansion (`GAME.EXE` `0x80067c10` and `0x80077ad8`, `OPEN.EXE`
`0x80020294` start 0x7c bytes above their frames). ASPSX also spells `li`
as `ori`, which the carves accept as ordinary code.

Over the full structural census (library text included), every decoded
`jal` target lands on a start except eight interior entries in the Psy-Q
text (GTE assembly with fall-through entries at `GAME.EXE` `0x80074d88`,
`0x80074d90`, `0x80075190`, `0x800756b0`, `0x80075b50`, `0x800776c8`, and
`EnterCriticalSection` behind exception code at `OPEN.EXE` `0x8001ad2c` and
`END.EXE` `0x800172f8`), which stay unsplit. All 62 distinct FDAT-to-`GAME.EXE`
call targets and all 32 handler slots of every module land on starts. The
short code-pointer tables some modules keep between `0x88` and the default
handler point at labels inside one function (switch cases), not at starts.

### Psy-Q library text: pending

The library text is not admitted yet: its bytes stay `unclassified` gaps
whose note says so. The bands are

| Image | Library text |
| :---- | :----------- |
| `PSX.EXE` | `0x80010118..0x80010248` |
| `GAME.EXE` | `0x800144f0..0x80014b48`, `0x80061be4..0x8007e4c4` |
| `OPEN.EXE` | `0x800136c0..0x800136d8`, `0x800144f4..0x8002c1b8` |
| `END.EXE` | `0x80011cfc..0x80011d14`, `0x8001240c..0x800232a0` |

The game bands end where `ghidra_psx_ldr` signature sets 340/330 begin to
match (`NONE2`, `MALLOC.OBJ`, LIBCARD's BIOS stubs, LIBCD, ...), and no
game-band function has the library's delay-slot epilogue; the game's
`0x80014b14` (CARD.OBJ `_card_clear`, calling `_new_card`/`_card_write`) and
`0x80061bc4` were placed by their epilogue and callees. `GAME.EXE` and
`OPEN.EXE` link Library 3.4 members (intr.c 1.73, sys.c 1.116), `END.EXE`
Library 3.3 ones.

TODO(sdk): once the toolchain pins the Library 3.4 and 3.3 archives:

1. run `kf-function-audit --exe-dir retail` and `kf-function-propose` to a
   fixed point and admit the library-band starts the game-band filter held
   back (about 1,390; the three lone 8-byte "returns" the audit carves in
   data tables at `GAME.EXE` `0x80094c34`, `OPEN.EXE` `0x8003d640` and
   `END.EXE` `0x80034714` are not code);
2. run `kf-vendored-seed --exe-dir retail --sdk-lib-dir <3.4 LIB>` (and the
   3.3 archives for `END.EXE`) so FID and exact rows carry the right release,
   with signature sets 340 then 330 as fallback; and
3. seed identities only for the starts left non-vendored.

## Ownership board

`kf verify board` reports **unresolved data ownership** separately from the
informational **raw DAT_ identities** count. An identity has an established
owner when its complete extent is covered by an image- and storage-matching
source `RODATA`/`DATA` claim, or by a supported/proven identity with an owner
and recorded evidence. A name alone, a candidate annotation, address proximity,
or a pointer into SDK code does not establish ownership. SDK-owned data follows
the same evidence rule. Counts describe inventory candidates, not necessarily
the number of original C objects.

Both data inventory counts are informational: discovering new data must not
fail the source cleanliness ratchet. `--update` saves only ratcheted metrics.
For the remaining work list, run `kf verify board --data`; use
`kf verify board --data all` to include each accepted owner and its evidence.
These commands read curated inputs and source claims without binary heuristics
or changes to the inventory.

## Review loop

```sh
kf-function-audit --exe-dir retail      # build/function-audit/candidates.tsv
kf-function-propose                     # build/function-audit/proposed-retail/
kf-vendored-seed --exe-dir retail       # build/vendored-seed/functions_vendored.tsv
kf inventory check
kf inventory propose
kf inventory ghidra --image game --image open
kf inventory propose-ghidra

kf sema --image game addr 0x80014bd4
kf sema --image fdat_005 xref 0x801e8398 --confirmed-only
kf sema --image game disasm 0x80014bd4 --blocks
```

Every generator writes below `build/`; diff its output against
`config/retail` and admit rows by hand.

`kf inventory propose` writes ignored review files:

- `build/function-inventory/evidence.tsv` combines MIPS live-in signature
  hints, callers, callees, indirect calls, strings, data references, and basic
  instruction-shape counts for every non-vendored function;
- `build/function-inventory/data-evidence.tsv` lists confirmed/candidate users,
  read/write/address counts, and tentative scope/owner hints for every tracked
  datum;
- `build/function-inventory/relocs-bss.tsv` proposes decoded MIPS HI/LO pairs
  from non-vendored code to admitted BSS extents.

A datum with one user receives a `function-static` hint and that function as
an owner hint; the adjacent confidence column distinguishes confirmed from
candidate xrefs. This is prioritization evidence only: another xref may still
be missing, and local linkage cannot be reconstructed from one use alone.
Multiple users produce `shared`, not `global`: without reliable TU ownership,
xrefs cannot distinguish external linkage from a file-local static used by
several functions.

`kf inventory ghidra` keeps persistent, ignored projects under
`build/ghidra-inventory`. Its JSON contains the inferred prototype, structured
parameter storage, decompiled C, and non-flow memory references for each
non-vendored function. Ghidra results are proposals because analysis can merge
functions, infer the wrong width, or manufacture a misleading default name.
They must be checked against retail instructions, delay slots, callers,
callees, relocation referents, and the pinned headers before admission.
`kf inventory propose-ghidra` merges those inferred prototypes into an ignored
review TSV while preserving supported/proven signatures; it also never edits
the curated identity file. It also writes `ghidra-data-xrefs.tsv`, grouping
Ghidra's memory references by target and marking each as admitted data/text,
scratchpad, hardware, or unowned RAM. Unowned RAM and zero-admitted-site groups
are queues for missing global/static extents and relocation review.
Its companion `data-identities.ghidra.tsv` proposes non-overlapping BSS extents
only for RAM addresses directly read or written by an instruction. Mere scalar
constants, pointer-only parameters, heap initialization addresses, scratchpad,
and hardware references are not auto-admitted.

## Curation rules

- Never key an identity by VA alone; `image` plus `va` is the key. The FDAT
  modules all share one window, and their calls into `GAME.EXE` are
  cross-image references.
- Preserve an address-derived name until evidence supports a semantic one.
- Record semantic argument names only after checking call sites and the body.
- Do not create an interior global when the address is a field or element of a
  larger owned object; refine the owner's datatype and extent instead.
- `scope=static` means file-local linkage and `scope=function-static` means a
  function-local static. Neither may be promoted solely from xref count.
- Keep generated reports under `build/`; only reviewed edits belong in the two
  identity TSVs.

The inventories are intentionally iterative. Missing functions, relocations,
data extents, and xrefs can invalidate earlier candidates without invalidating
the workflow.

Safe delinking withholds an out-of-load pair until its relocation row is
individually reviewed; candidate rows also cannot suppress a stronger
reviewed pair at the same instruction.
