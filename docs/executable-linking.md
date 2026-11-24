# Source-to-EXE build

Inside `nix develop`, run:

```sh
kf build
```

`kf link` is an alias for the same executable builder. There is one candidate
artifact chain (tools and provenance in [`toolchain.md`](toolchain.md)):

```text
C source -> cpppsx-260 / CC1PSX.EXE (GNU C 2.6.0) -> assembly -> ASPSX 2.21
         -> [DMPSX 2.03] -> Psy-Q OBJ
Psy-Q OBJ + Runtime Library 3.3 OBJ/LIB -> PSYLINK 2.27 -> CPE -> CPE2X 1.5 /CA -> PS-X EXE
```

The EXE is the unchanged output of CPE2X. Retail bytes are comparison inputs
only. The build does not synthesize object sections, force addresses from
`DATA()` claims, rewrite the CPE, patch the EXE, or copy missing retail bytes.
A failed phase removes the stale EXE. Each image's `build.json` records every
tool, input hash and command, and whether its layout is proven.

DOSBox-X runs every DOS tool at a fixed emulated rate (`DOS_CYCLES` in
`scripts/psxbuild/sdk.py`), so DOS stack residue cannot vary with host load,
and `nix flake check` (`executable-determinism`) builds every linked program
in two trees and requires identical CPE and EXE bytes. CPE2X 1.5 writes every
header field itself.

A partial image, one whose source units claim some but not all of its
admitted game functions, is not linked: its code still calls and reads game
code no source provides, which PSYLINK cannot resolve. `kf build` reports it
as `partial` with the number of unclaimed game functions and does not fail;
its units are verified by the target relink and data gates of `kf check`.
Sources that claim no retail function, such as the synthetic link controls,
are always linked.

## Link model

PSYLINK, not the build script, places SDK library members. The build only
chooses the explicit objects, their order and the `inclib` order
(`IMAGE_LINKS` in `scripts/psxbuild/link.py`):

- Archives are visited in `inclib` order, so the groups of members from each
  archive follow one another in the linked text in that order.
- After it takes a member, the linker searches again from the first archive,
  so a dependency in an archive listed earlier follows the member that needs
  it directly.
- Every program uses one text group (`.rdata`, `.text`, `.data`, `.sdata`)
  and one bss group (`.sbss`, `.bss`); `PSX.EXE`'s string literals at
  `0x80010000` show `.rdata` leads.

| Image | Explicit objects | `inclib` order | Status |
| --- | --- | --- | --- |
| `PSX.EXE` | units, then `2MBYTE.OBJ` | LIBAPI, LIBSN | proven: byte-identical |
| `GAME.EXE` | `NONE2.OBJ`, `MALLOC.OBJ`, then units | LIBCARD, LIBCD, LIBSPU, LIBSND, LIBPRESS, LIBGTE, LIBETC, LIBAPI, LIBC, LIBGPU | unproven |
| `OPEN.EXE` | `open.main`, `NONE2.OBJ`, `open.movie`, `open.card`, `MALLOC.OBJ` | LIBAPI, LIBC, LIBPRESS, LIBGPU, LIBGTE, LIBCD, LIBETC, LIBSND, LIBSPU, LIBCARD | objects in retail order; libraries differ |
| `END.EXE` | `end.main`, `NONE2.OBJ`, `end.movie`, `MALLOC.OBJ` | LIBAPI, LIBC, LIBPRESS, LIBGPU, LIBGTE, LIBCD, LIBETC, LIBSND, LIBSPU | objects in retail order; libraries differ |

The GAME row transcribes the Ghidra signature census of the scoping study
(NONE2, MALLOC and a `_card_clear` CARD object ahead of GAME's game code) and
has not been linked against retail. GAME and OPEN were built with Library 3.4
and END with a LIBGPU `SYS` and several LIBSND members newer than the 3.3 CD;
those objects are not preserved, so no overlay can rebuild byte-identically
from the pinned archives. GAME's CARD object is not modelled.

OPEN and END are fully claimed and link. Their explicit objects land in the
retail order and every game function is exact, so each linked image matches
retail up to the first library member whose release differs. In END that is
LIBSND: the text agrees in order and size through LIBGPU, LIBGTE, LIBCD and
LIBETC, shifted only by 0x28c bytes of extra `.rdata` that the 3.3 LIBSND
members bring (switch tables of members the newer LIBSND does not pull in),
and from `SsInit` on the 3.3 members are larger. In OPEN the shift starts at
LIBPRESS (3.4's `DecDCTGetEnv` and DECVLC), and 3.4's LIBCD and LIBGPU `SYS`
1.116 follow. `kf link --compare-only` scores both near 78% island-aligned.
OPEN's LIBCARD stubs are its last text members, after LIBSPU. The units'
COMMON requests (`cdb`, `db` and the primitive buffers) cannot be proven at
their retail addresses: PSYLINK orders COMMON by symbol-hash bucket among the
SDK's own COMMON, and the 3.3 archives supply a different SDK set; that
placement is a recorded `common-placement` exemption in
`config/check_exemptions.tsv` (see the delinking document's Validation).

## PSX.EXE

`src/psx/main.c` (one unit, `psx.main`, profile `gcc260-o2-g8`) links with
`2MBYTE.OBJ`, which supplies `__main` (`0x80010118`) and the entry point
`__SN_ENTRY_POINT` (`0x80010120`), then LIBAPI's `EnterCriticalSection`,
`SetMem`, `_96_init`, `InitHeap`, `Load`, `Exec` and `_96_remove` stubs and
LIBSN `SNDEF`'s `_stacksize`/`_ramsize` words. The result equals the retail
`SLUS_002.55` (4096 bytes, sha256 `c8e696c3…ae70`), header included: CPE2X
1.5 `/CA` writes the "North America area" title, the `801ffff0` stack base
and zero reserved words. PSYLINK 2.37 links the same objects to a different
file.

### Runtime smoke test

`kf-run-candidate` substitutes linked executables into a copy of the raw retail
Mode 2 disc (`KF_CANDIDATE_IMAGES` selects which ones, for controlled hybrid
tests). The retail BIN must match the disc SHA-256 in `config/images.toml`.
Every changed sector's EDC and P/Q ECC is regenerated; `kf-run-retail` boots
the unmodified disc.

Existing source-to-EXE outputs can be compared again without rebuilding them:

```sh
kf link --compare-only
```

## Layout-tolerant executable comparison

`kf link --compare-only` compares moved byte regions in existing outputs and
refreshes the second generated README block, `executable-score`:

```sh
kf link --compare-only
```

The command accepts `--image` with any image key; without it, images that
have no linked EXE yet are skipped. The README keeps one row per image
and a byte-weighted total for the available images. It reads saved reports
only when their candidate hashes match EXEs still on disk and their retail
hashes match the configured retail identities. Missing, changed or failed
outputs receive no cached score. The executable and function blocks share
the same update lock and preserve each other.

The **Data modules exact** column shows complete passing source data owners
out of all compared owners, using the same strict gate as `kf verify data`.
A module counts only when every initialized section, relocation, BSS layout
and retail section placement passes. Exact functions or equal initializer
bytes alone do not make its data exact. Separate config-owned SDK contributions
are excluded; these object checks do not measure the generated EXE's similarity.
`kf analyze`/`kf check` and `kf bank` also refresh this column. Missing comparison
artifacts or objects older than their source, headers, inventories or build
inputs show `—`, independently of executable-score availability.

The report is a content heuristic called `unique-byte-islands-v1`, not a
reimplementation of objdiff or a claim of function identity:

1. Find exact 16-byte windows that occur once in each load area. Coalesce
   overlapping windows with the same displacement, then select the longest
   nonoverlapping runs. Each byte can be assigned at most once in each image.
2. Extend selected runs through equal, unused neighboring bytes. Also keep
   exact file-boundary prefixes and suffixes, so repeated padding does not
   require a unique seed.
3. Join adjacent anchors only when they are neighbors in both images. Each
   intervening gap is at most 128 bytes. A bounded byte-sequence comparison
   accepts gaps with at least 60% similarity, or edits of at most 16 bytes
   per side when the two flanking exact runs supply at least 85% similarity
   across that local span. Unchanged bytes inside an accepted gap earn
   credit; its edited, inserted or deleted bytes do not.
4. Sum equal byte pairs across all selected islands. Report similarity as
   `200 * equal_pairs / (retail_load_bytes + candidate_load_bytes)`. All
   unpaired bytes remain in the denominator. The nonzero score uses the
   same formula with zero-valued bytes removed from both counts.

Islands may move or reorder, but no address, opcode, register, immediate,
constant or data byte is masked. A changed pointer still loses byte credit;
similar pointer encodings do not prove the same referent. Short or repetitive
regions without suitable anchors can remain unpaired even when shared.
The algorithm is conservative and deterministic, rather than an optimal
global alignment or an instruction-semantic comparison.

The island scores cover the complete EXE load areas, including sector
padding, and exclude the 2048-byte headers. The separate fixed-offset
comparison still covers the complete files and is the only executable
equality check. Neither island similarity nor island coverage changes
function scores, banking criteria or the full data-layout gates.

Normal builds write tool commands and input hashes into each image's
`build.json`. They do not load retail EXEs, compare output bytes, or update
the README. Explicit comparison-only runs write `fuzzy-comparison.json` and
`fuzzy-comparison.html`, preserving the build report and executable.
The standalone HTML contains a movement
map, filterable islands with both addresses and sizes, and the largest
unanchored regions. The JSON retains all islands, complementary unanchored
ranges, parameters, counts and input hashes. These generated files stay
under `build/link/{psx,game,open,end}/` and are not committed.

## Verification and limits

`PSX.EXE` rebuilds byte-identically from source (`kf build`, then compare
`build/link/psx/PSX.EXE` with `retail/PSX.EXE`, or `kf link --compare-only`).
That proves the compiler, flags, assembler, linker, converter and the PSX
library objects for that program; it executes no game code.

The native executable controls (`tests/test_executable.py`) build small C
programs through the same chain: the `__main` call and `2MBYTE.OBJ`
placement, the CPE2X 1.5 header, unchanged original archives, each overlay
model's startup and `inclib` order, exclusion of an 8192-byte BSS buffer, and
rejection of unresolved references or stale output.
`tests/test_dmpsx.py` checks the DMPSX rewrite of INLINE.H GTE macros.

`psx.main` is exact and banked: `src/psx/main.c` claims its data (the path
table, `program_index`, `next_program` and the EXEC header) and
`relocs.tsv` holds its reviewed relocations, so objdiff scores it 100%,
`kf verify data` and the target relink pass, and `kf check` passes for
`PSX.EXE`. Without the data claims objdiff scored it 98.89% even though the
EXE was byte-identical: the delinked retail object then had no owners for
those references, so its relocation referents differed from the native
object's symbols.
