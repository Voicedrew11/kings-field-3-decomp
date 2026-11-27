# SDK and source-to-EXE toolchain

The executable chain is a 1995 Psy-Q chain: Sony's GNU C 2.60 compiler and
the assembler, linker, converter, GTE post-processor, libraries and headers of
Sony's Runtime Library 3.3 CD. It rebuilds `PSX.EXE` byte-identically. Each
statement below says whether it is **proven** (a byte-identical rebuild or an
exact object match against retail), or a **probe** (the closest available
choice, not yet reproduced). How this chain turns C into retail's
instructions, and the source forms behind them, is in
[compiler-notes.md](compiler-notes.md).

| Part | Pinned file | Source | Status |
| --- | --- | --- | --- |
| Compiler | `CC1PSX.EXE`, "GNU C 2.6.0 [AL 1.1, MM 40] Sony Playstation" (sha256 `6587ed37…`) | Sony "GNU C Compiler Version 2.60 (World)" floppy, `CC1PSX.EXZ` dated 1994-10-07 | proven: `PSX.EXE` and all 28 FDAT modules rebuild byte-identically |
| Compiler (GAME) | `cc1psx-260r`: GNU C 2.6.0 built from the FSF source with the Decompals `gcc-2.6.0-psx` recipe and FSF's 1994-08-07 reload change | `nix/gcc260r.nix`, `patches/gcc-2.6.0-reload-19940807.patch` | **reconstructed revision**: no binary preserved; see [GAME's compiler revision](#games-compiler-revision) |
| Flags | `-O2 -G8` (the compiler's own `-G` default), or `-O2 -G0` | profiles `gcc260-o2-g8`, `gcc260-o2-g0`; for GAME `gcc260r-o2-g8`, `gcc260r-o2-g0` | proven per unit that matches (e.g. `psx.main`; `-G0` for the FDAT modules) |
| Assembler | ASPSX 2.21 (1995-06-22, `b52bf3d4…`) | Runtime Library 3.3 CD `PSYQ/ASPSX.EXE` | proven |
| GTE post-processor | DMPSX 2.03 with `INLINE.TBL` | Runtime Library 3.3 CD `PSX/BIN` | proven (GAME GTE functions exact, e.g. `0x80035394`, `0x8003bb04`, `0x800400ac`) |
| Linker | PSYLINK 2.27 (1995-08-30, `39c9e16b…`) | Runtime Library 3.3 CD `PSYQ/PSYLINK.EXE` | proven for `PSX.EXE` and the 28 FDAT modules |
| Converter | CPE2X 1.5 `/CA` (1995-07-07, `777caca8…`) | Runtime Library 3.3 CD `PSXGRAPH/BIN/CPE2X.EXE` | proven (header byte-identical) |
| Libraries | Library 3.3: `PSX/LIB` archives, `2MBYTE.OBJ`, `NONE2.OBJ`, `MALLOC.OBJ` | Runtime Library 3.3 CD | proven for `PSX.EXE`; probe for GAME, OPEN, END |
| Headers | `PSX/INCLUDE` (LF copy in `include-lf/`) | Runtime Library 3.3 CD | `INLINE.H` and `GTEMAC.H` proven by the GTE functions; probe otherwise |

Both media are Archive.org `ps1_sdks` items, fetched by SHA-256 in
`nix/psx-toolchain.nix`: the floppy image
`GNU C Compiler Version 2.60 (World) (Disk 1).img` and the redump
`Programmer Tool - Runtime Library Version 3.3 (Japan)_DTL-S2190_redump.zip`.
Only data track 1 of the CD is read (`nix/extract-iso9660.py`), only the
files above are copied, and each is hash-checked. The CD's `MALLOC.OBJ` is
byte-identical to the Release 2.5 allocator the King's Field projects pinned,
so the Release 2.5 floppies are no longer fetched.

## Evidence

**PSX.EXE.** `src/psx/main.c` compiled by the floppy's `CC1PSX.EXE` with
`-O2 -G8`, assembled by ASPSX 2.21, linked by PSYLINK 2.27 with `2MBYTE.OBJ`,
LIBAPI and LIBSN, and converted by `CPE2X /CA`, equals the retail 4096-byte
file (sha256 `c8e696c3…ae70`). Facts that discriminate:

- The compiler calls `__main` at the start of `main`, which `2MBYTE.OBJ`
  supplies (retail `0x80010118`). The 2.7.2-cdk rebuild used by Shadow Tower
  emits no such call.
- `main`'s frame (`ra` at 24, `s1`/`s0` holding the path table and the EXEC
  header) and its filled `jal __main` delay slot.
- The floppy binary is the one the mkst/esa `psyq3.3`/`psyq3.5` bundles
  carry. The Psy-Q 3.0 kit's `COMPILER/CC1PSX.EXE` (sha256 `e65635ec…`) is a
  different file with the same version string; it emits identical assembly
  for `main` and for a 1,800-line control corpus, so the samples cannot tell
  the two apart. The floppy is pinned as Sony's own compiler distribution.
- Small globals are reached `$gp`-relative, so the unit is `-G8`.
- PSYLINK 2.37 (Psy-Q 3.5) links the same objects to a different file;
  PSYLINK 2.27 reproduces it.

**GAME samples.** Scratch compiles of four GAME functions matched retail
exactly, relocation fields aside, before any GAME unit existed (all four are
now banked exact in their units): the mult/div leaf at
`0x8004c5ec`, the `kf_PlaySound(a, b, b, 0)` wrapper at `0x8001576c`, the
four-call sequence at `0x80015064`, and the frame gate at `0x80019614`
(`while (counter < 4) VSync(0); counter = 0;`). The frame gate settles the
address form the scoping study left open: retail builds the counter's full
address with `lui`/`addiu`, loads through `0(reg)` and keeps the address in
`s0` across the `VSync` loop. GNU C 2.6.0 emits exactly that when the counter
is a member of a `volatile` aggregate (an array element or a struct member
give the same code); a plain `volatile` scalar compiles to the `lw sym`
macro form instead. The 2.7.2-cdk rebuild needed `%hi`/`%lo` for the same
shape. The counter is the `frame_gate` member of the CD state
`cdrom_ctl` (`include/game/cd.h`), which the CD module reaches as one
aggregate.

**ASPSX.** On GAME's mult/div leaf at `0x8004c5ec`, ASPSX 2.21 assembles the
2.6.0 output to retail exactly (the `div` with its `break 7`/`break 6`
checks, `li` as `ori`). ASPSX 2.30 and 2.34 insert a hazard `nop` before
the `div`; 2.05 and 2.08 differ there too and cannot address small data
`$gp`-relative, which `PSX.EXE` needs. ASPSX 2.21 declares empty
`.ctors`/`.dtors` sections and writes a `$gp` offset as patch 30,
`X - sectstart(.sdata)`; the ELF view reads it as `R_MIPS_GPREL16`.

**Libraries.** RCS strings place GAME and OPEN on Library 3.4 (LIBETC
`intr.c` 1.73 1995-11-10, LIBGPU `sys.c` 1.116 1995-12-01, LIBCD `bios.c`
1.71 1995-12-01) and END on Library 3.3 (`intr.c` 1.71 1995-08-29) but with
LIBGPU `sys.c` 1.109 (1995-10-26). The 3.3 CD carries `intr.c` 1.71 and
`sys.c` 1.107. Whole-object matches of `.text` with relocation fields masked:

| Program | SDK bytes in the census | Exact Library 3.3 objects | Exact Library 3.5 objects |
| --- | ---: | ---: | ---: |
| `END.EXE` | `0xa4ec` | `0x8bfc` | `0x1324` |
| `OPEN.EXE` | `0x10898` | `0x7544` | `0x1cb0` |
| `GAME.EXE` | `0x17640` | `0xc53c` | `0x4a08` |

END's misses are LIBGPU `SYS` (near-identical code, newer revision) and a
group of LIBSND members (`SSINIT`, `SCSMVOL`, `SCSSVOL`, `VM_VIB`, `SCSNCK`,
`VM_VSU`, `SCSSATTR` and the `SpuVm`/`SsUt` volume members) that neither 3.3
nor 3.5 contains. No Library 3.4 archive, and no source of END's newer
members, was found: Archive.org `ps1_sdks` holds Runtime Library 2.0, 2.6,
3.0, 3.3, 3.5, 3.6, 4.0, 4.1, 4.3, 4.4 and 4.6; the other public Psy-Q
collections searched (Archive.org `psyq-sdk`, psx.arthus.net, the
archaic-toolchains, FoxdieTeam, cdlewis and mkst/esa repositories) start at
3.3 or later and none is 3.4. The 3.3 archives are therefore the closest
proven set and stay a labelled probe for GAME, OPEN and END: those programs
cannot link byte-identically from them. `kf-vendored-seed` and
`kf-fid-census` read the same archives; the per-function result (3.3
object, 3.3 function ID, signature only, unmatched version) is in
[the inventory](function-and-data-inventory.md#psy-q-library-text).

## GAME's compiler revision

GAME's units compile with `cc1psx-260r`, which is **not a preserved
compiler**: it is GNU C 2.6.0 rebuilt from source with one FSF change from
after the 2.6.0 release, chosen because retail GAME shows its effect and
nothing else distinguishes it from the pinned compiler. The evidence:

- **What 2.6.0 cannot emit.** `0x8003daec` and `0x800533e8` spill a register
  parameter as a word and read its low half back from the slot (`sw a1,16(sp)`
  ... `lhu s1,16(sp)`; `sw a3,64(sp)` ... `lhu v0,64(sp)`). GNU C 2.6.0's
  reload always reloads the whole word for such a read (`lw t0,slot`, then a
  `move`/`andi` of `t0`); both Sony 2.6.0 binaries (the floppy's and the Psy-Q
  3.0 kit's `CC1PSX.EXE`) do. FSF's change of 1994-08-07 (Kenner; `reload.c`
  `push_reload` and `find_reloads`: the `LOAD_EXTEND_OP` tests force a reload
  only for paradoxical subregs; `reload1.c` `eliminate_regs`: a `USE` of the
  pseudo before the insn) reads the narrow part directly. A scan of every
  function with a frame in GAME, OPEN, END and the FDAT modules finds the
  2.6.0 form nowhere and the later form only in these two functions.
- **The patch.** `patches/gcc-2.6.0-reload-19940807.patch` holds only the hunks
  of that ChangeLog entry that exist for this target, copied from GNU C 2.6.3
  (no later entry touches those lines). For MIPS it reduces to `!=` becoming
  `>` in two tests and the `USE` insn.
- **The build.** `nix/gcc260r.nix` follows the Decompals
  `gcc-2.6.0-psx.Dockerfile` (old-gcc `b74211c`, already pinned for 2.5.7)
  on the FSF 2.6.0 tarball. Without the patch (`.#gcc260SourceBuild`) it emits
  assembly identical, file for file, to `cc1psx-260` (the Decompals 0.17
  binary) for all 75 C units; with it (`.#cc1psx260r`) 73 units are identical
  and in `render.c` and `object4c.c` only `0x8003daec` and `0x800533e8`
  change. Both builds are bit-reproducible (`nix build --rebuild`).
- **Against retail** (`kf try` listings, every claimed function, 1,076):
  every function that is exact under `CC1PSX.EXE` stays exact, with the same
  listing, under `cc1psx-260r` (GAME, OPEN 34, END 13, PSX 1, FDAT 437).
  GAME gains `0x8003daec` (with the newest `render.c`), `0x800533e8` (with
  its fourth parameter an `int`, passed straight to the callees) and the
  `.text` addend of `0x80040830`, which follows `0x8003daec`'s size. No other
  function's listing or score moves.
- **Why not 2.6.3.** The Decompals `gcc-2.6.3-psx` rebuild also makes both
  functions exact but changes the frames of `0x80047010` (304 bytes against
  retail's 288) and `0x800568bc` (736 against 696), most likely through
  2.6.3's `function.c` handling of address-taken temporaries (ChangeLog
  1994-10-06 on). Of the 1,076 claimed functions these four are the only
  ones 2.6.0 and 2.6.3 compile differently (FDAT, OPEN, END and `PSX.EXE`
  are identical). Retail therefore behaves like FSF sources after 1994-08-07
  and, probably, before 1994-10-06. No FSF release falls in that window
  (2.6.1 is 1994-11-01), so the compiler was presumably a Sony build of that
  period's sources that is not preserved; GAME links Library 3.4, whose kit
  is not archived either.

`PSX.EXE`, the FDAT modules, OPEN and END keep the floppy's `CC1PSX.EXE`:
they are proven with it, and every function of theirs compiles identically
under both compilers, so they cannot tell the two apart. If a Sony binary
with this reload is ever found, it replaces `cc1psx-260r`.

## Build commands

There is one source compilation path:

```text
C source -> cpppsx-260 (CCPSX 1.17 definitions) -> CC1PSX.EXE (DOSBox-X)
                                                   [GAME: cc1psx-260r, native]
         -> ASPSX 2.21 -> [DMPSX 2.03 if the unit uses INLINE.H] -> Psy-Q OBJ
Psy-Q OBJs + 2MBYTE/NONE2/MALLOC + Library 3.3 archives -> PSYLINK 2.27
         -> CPE -> CPE2X 1.5 /CA -> PS-X EXE
```

`compiler = "gcc260-sony"` runs the floppy's `CC1PSX.EXE` under DOSBox-X.
The Decompals GCC 2.6.0 rebuild's `cpp` preprocesses with the definitions the
Psy-Q CCPSX 1.17 driver passes (`mips`, `__psx__`, `_MIPSEL`,
`__CHAR_UNSIGNED__`, `LANGUAGE_C`, `__OPTIMIZE__`, …); its `cc1` answers only
the auxiliary sizeof probe. Both are optimizer-independent. The floppy's
`CPPPSX.EXE` is pinned beside the compiler but not run. Plain `char` is
unsigned in this `CC1PSX` (`*p` of a `char *` loads with `lbu`), which is
what the driver's `__CHAR_UNSIGNED__` announces.

`compiler = "gcc260r-native"` runs `cc1psx-260r` natively on the same
preprocessed source (the same driver definitions) with the same flags, and
hands its assembly to the same ASPSX step.

`kf build`, `kf analyze` and `kf try` share the same compiler, assembler and
DMPSX step. Runtime Library 3.3's `INLINE.H` writes each GTE operation as
placeholder words (`.word` values below `0x2000` with SPECIAL funct `0x3e` or
`0x3f`) that DMPSX rewrites in the object into COP2 transfers and commands.
A unit whose assembly carries such a word runs DMPSX directly after ASPSX, and
the result must differ from ASPSX's object only inside those words. Other
objects never reach DMPSX.

Debug metadata is enabled in the compiler and assembler so private symbols
remain inspectable. It does not reach the linked executable.

| Environment variable | Content |
| --- | --- |
| `PSYQ_CC1PSX` | the floppy's `CC1PSX.EXE` |
| `PSYQ_ASPSX`, `PSYQ_PSYLINK`, `PSYQ_CPE2X`, `PSYQ_DMPSX` | the 3.3 CD tools (`INLINE.TBL` beside DMPSX) |
| `PSYQ_LIB` | the 3.3 CD `PSX/LIB` |
| `PSYQ_INCLUDE` | the 3.3 CD `PSX/INCLUDE` with LF line endings |
| `PSYQ_MALLOC_OBJ` | the 3.3 CD `MALLOC.OBJ` |
| `PSYQ_SDK`, `PSYQ_BIN`, `PSYQ_COMPILER` | the Psy-Q 3.0 kit (analysis corpus only) |

## Profiles

`config/units.toml` keeps `gcc260-o2-g8` and `gcc260-o2-g0`, the latter for
translation units whose globals are all reached through `lui`/low pairs
(every FDAT module, `game.save_data`); each is proven for the units that
match under it: 95 GAME, 20 OPEN and 5 END game functions use `$gp`
and the rest do not, so both settings occur. GAME's units use
`gcc260r-o2-g8` and `gcc260r-o2-g0`, the same settings with `cc1psx-260r`
([GAME's compiler revision](#games-compiler-revision)). The King's Field II probe
profiles (GCC 2.5.7/2.6.0 rebuilds with ASPSX 1.07) are removed; the
`gcc257-native`, `gcc260-native` and `gcc241-kit` compilers stay selectable.

## Open questions

- The overlays' link layouts in `scripts/psxbuild/link.py` come from the
  signature census, not from a link.

## Psy-Q 3.0 kit

The complete hash-pinned Psy-Q PS-X Development System Release 3.0 kit
(Archive.org `psyq_psx_toolchain_april_08_1994/psx.zip`, files dated
1995-04-08) is inherited from the King's Field II project. KF3 links none of
it. It remains the analysis corpus of `kf-vendored-seed` and `kf-fid-census`
and supplies the `gcc241-kit` probe compiler. `scripts/create-toolchain.py`
verifies and stages it under `psyq-3.0/` with a deterministic manifest.

No generated SDK binary, retail game image, or executable belongs in Git.

## Analysis tools

`nix develop` provides the executable chain and the separate analysis
programs, plus Ghidra/PyGhidra, DOSBox-X, little-endian MIPS GNU binutils,
psy-k, disc-image utilities, objdiff, and the normal C/C++/Python build
tools. Ghidra plugin packaging is documented separately in
[`ghidra.md`](ghidra.md).

Splat, Rabbitizer, and spimdisasm use the separately locked Python environment
because the latter two are not available in the pinned Nixpkgs revision. The
shell wrappers keep both setup and execution on that project lock:

```sh
kf-python-sync
splat --help
```

The first command is the one-time `uv sync --frozen --no-install-project`
step; `splat` delegates to `uv run --frozen` thereafter.
