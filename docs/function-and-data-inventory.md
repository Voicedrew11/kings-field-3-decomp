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

| Image | Game functions | `proven` | `validated` | `candidate` | Bytes |
| :---- | -------------: | -------: | ----------: | ----------: | ----: |
| `PSX.EXE` | 1 | 1 | 0 | 0 | `0xe0` |
| `GAME.EXE` | 593 | 537 | 0 | 56 | `0x4d09c` |
| `OPEN.EXE` | 34 | 33 | 1 | 0 | `0x26c8` |
| `END.EXE` | 13 | 12 | 1 | 0 | `0xa3c` |
| 28 `FDAT_nnn.BIN` | 437 (14–22 each) | 10 | 0 | 427 | `0xb208` |
| **Total** | **1,078** | **593** | **2** | **483** | |

These are the denominator. `functions.tsv` also holds the four PS-X EXE
entry rows (Psy-Q startup objects) and the 1,383 Psy-Q library functions
(below); all of those have `functions_vendored.tsv` rows. Game identities
were seeded address-only with candidate signatures from the static MIPS
live-in heuristic (`kf inventory seed`) and are refined as sources land;
`data_identities.tsv` holds 175 rows across the images, from `PSX.EXE`
`main`'s four (`program_paths`, `program_index`, `next_program`,
`program_header`) to the GAME, OPEN, END and FDAT globals the units define.

### Evidence tiers

Each row's note starts with its AGENTS.md tier (library-band rows with
`Psy-Q library band; ` first). `tier=proven` is decoded direct control
flow: the PS-X EXE entry, or the target of a `jal` or a cross-function `j`
decoded from carved code, including the 52 `GAME.EXE` functions that FDAT
modules call directly. `tier=candidate` is everything else: 412 FDAT rows
are named by their module's dispatch table (stored pointers that
`GAME.EXE` calls indirectly), and the rest are carved from frame or return
shape alone. `tier=validated` needs a reviewed relocation row that passes
the safe delinker's checks: END's and OPEN's `strCallback`, which nothing
calls directly, are validated by the reviewed `lui`/`addiu` pair in
`strInit` that passes them to `DecDCToutCallback`.

### Function starts

Starts come from `kf-function-audit` and `kf-function-propose`, rerun until
the proposal stops changing (three passes from the entry-only census). Each
row's `confidence` names the rule that admitted it:

- `entry`: the PS-X EXE header pc0;
- `mips-frame-carve`: a stack frame with `ra` saved and a matching stack
  restore at the return. The game code's GCC 2.6 always restores the stack
  before the return (`lw ra; addiu sp; jr ra; nop`); the library text has
  that form and the delay-slot one (`jr ra; addiu sp`), so only the
  converse marks library code. Loads scheduled above the frame allocation
  start at the previous function's return;
- `mips-return-carve`: valid code between one return (or the end of an
  overlay's 0x88-byte handler header) and the next return;
- `mips-call-carve`: a `jal` target decoded from admitted code that lies in
  unowned bytes, carved to the first unconditional transfer at or past the
  furthest forward branch it has seen (a tail call to the next start still
  ends it);
- `mips-call-split`: a `jal` target inside an admitted extent whose preceding
  code cannot fall through into it; and
- `mips-handler-carve`: an FDAT dispatch-table pointer, carved the same way
  (the shared `jr ra; nop` default handler directly after each header).

An admitted extent that ends at a `jr reg` the CFG's switch prover proves a
table dispatch, with every case in the unowned bytes up to the next start,
grows over its cases (`switch-extend`): `GAME.EXE` `0x8003260c`,
`0x80047010` and `0x800568bc` own their case bodies (0x1504, 0x4514 and
0x5338 bytes, tables at `0x80011b0c`, `0x8001209c` and `0x80012ebc`).

A `jal` target up to 0x40 bytes of straight-line code above an admitted
start moves that start back (the row's note records it). ASPSX expands
every `div` with `break 7`/`break 6` traps whose branches only skip the
trap; such a run counts as straight-line, and each expansion widens the
window once (`GAME.EXE` `0x80067c10` and `0x80077ad8`, `OPEN.EXE`
`0x80020294` start 0x7c bytes above their frames). ASPSX also spells `li`
as `ori`, which the carves accept as ordinary code.

Over the full census (library text included), every decoded `jal` target
lands on a start except eight interior entries in the Psy-Q text (GTE
assembly with fall-through entries at `GAME.EXE` `0x80074d88`,
`0x80074d90`, `0x80075190`, `0x800756b0`, `0x80075b50`, `0x800776c8`, and
`EnterCriticalSection` behind exception code at `OPEN.EXE` `0x8001ad2c` and
`END.EXE` `0x800172f8`), which stay unsplit. All 62 distinct FDAT-to-`GAME.EXE`
call targets and all 32 handler slots of every module land on starts. The
short code-pointer tables some modules keep between `0x88` and the default
handler point at labels inside one function (switch cases), not at starts;
the one function there (`FDAT_029` `0x801e8390`, byte-identical to admitted
functions of `FDAT_014`, `FDAT_020` and `FDAT_080`) is carved from the
header end. The audit's three lone 8-byte "returns" in data tables
(`GAME.EXE` `0x80094c34`, `OPEN.EXE` `0x8003d640`, `END.EXE` `0x80034714`)
are not code and are not admitted.

### Psy-Q library text

`config/evidence/sdk_text_bands.tsv` lists the reviewed library bands:

| Image | Library text | Starts |
| :---- | :----------- | -----: |
| `PSX.EXE` | `0x80010118..0x80010248` | 8 + entry |
| `GAME.EXE` | `0x800144f0..0x80014b48`, `0x80061be4..0x8007e4c4` | 578 + entry |
| `OPEN.EXE` | `0x800136c0..0x800136d8`, `0x800144f4..0x8002c1b8` | 447 + entry |
| `END.EXE` | `0x80011cfc..0x80011d14`, `0x8001240c..0x800232a0` | 350 + entry |

The band edges are where `ghidra_psx_ldr` signature sets 340/330 and the
Runtime Library 3.3 object matches stop and start (`NONE2`, `MALLOC.OBJ`,
LIBCARD's BIOS stubs, LIBCD, ...); nothing in a band calls or jumps into
game code, and no game-band function has the delay-slot epilogue. The
game's `0x80061bc4` was placed by its epilogue and callees.

`kf-vendored-seed --exe-dir retail` identifies every band function, in
this order of precedence:

1. exact objects: `config/evidence/psyq_33_text_sections.tsv` and
   `psyq_33_complete_objects.tsv`, produced by
   `python -m scripts.kf.sdk_object_census --exe-dir retail` against the
   pinned Runtime Library 3.3 archives and re-verified by the seed (a
   function lying inside a relocation-masked exact `.text` match, or a
   small object whose bytes equal the image outright);
2. project-built function IDs from the same archives (`fid-sdk33`): the
   function's masked bytes equal one 3.3 function, even where its object as
   a whole is a newer revision (END's LIBGPU `SYS`, sys.c 1.109 against the
   CD's 1.107);
3. `ghidra_psx_ldr` signature set 340, then 330 (`psyq340-signature`):
   signature-only, no preserved archive behind it; and
4. the band list (`sdk-unmatched-version`, library and module
   `unmatched`): band code none of the above matches, recorded as vendored
   because it is library code of a release no archive preserves, with the
   identified neighbours in its note.

| Image | 3.3 exact object | 3.3 function ID | signature only | unmatched version | Total |
| :---- | ---------------: | --------------: | -------------: | ----------------: | ----: |
| `PSX.EXE` | 9 | 0 | 0 | 0 | 9 |
| `GAME.EXE` | 285 | 109 | 102 | 83 | 579 |
| `OPEN.EXE` | 179 | 109 | 80 | 80 | 448 |
| `END.EXE` | 233 | 51 | 8 | 59 | 351 |
| **Total** | **706** | **269** | **190** | **222** | **1,387** |

The totals include the four entry rows (`2MBYTE.OBJ` and `NONE2.OBJ`, all
exact). The unmatched-version rows are GAME/OPEN/END's LIBSND voice
manager, SEQREAD and VS_VH code, two END LIBGPU `SYS` functions, the CARD
`_card_clear` object at `GAME.EXE` `0x80014b14` and the exception code in
front of OPEN's and END's `EnterCriticalSection`. Library 3.5 (not pinned)
matches less than 3.3 in every image. Identities are seeded only for the
starts left non-vendored, which are exactly the game functions.

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
python -m scripts.kf.sdk_object_census --exe-dir retail  # build/sdk-objects/ (config/evidence inputs)
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
