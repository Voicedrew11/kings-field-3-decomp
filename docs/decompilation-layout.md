# Thirty-two decomps in one repository

This project is one reverse-engineering effort with 32 independent
decompilation targets: four linked programs and the 28 code modules of
`FDAT.T`.

| Target | Retail role | Load window | Game functions |
| --- | --- | --- | ---: |
| `PSX.EXE` | program sequencer (boot file `SLUS_002.55`) | `0x80010000..0x80010800` | 1 |
| `OPEN.EXE` | opening program | `0x80011000..0x80040800` | 34 |
| `GAME.EXE` | main game program | `0x80011000..0x8009c800` | 593 |
| `END.EXE` | ending program | `0x80011000..0x80037800` | 13 |
| `FDAT_nnn.BIN` (28) | area code modules loaded by `GAME.EXE` | `0x801e8308..` +`0x800`/`0x1000` | 436 |

`OPEN.EXE`, `GAME.EXE` and `END.EXE` occupy the same RAM window at different
times. They are not sections of one executable. The same virtual address can
name unrelated functions or data depending on which program is loaded.
`PSX.EXE` is a fourth linked program, not a header attached to any overlay:
its table at `0x8001024C` names `OPEN.EXE`, `GAME.EXE` and `END.EXE` by index,
and it loads the next one from the index byte at `0x800102F0`.

The FDAT modules are headerless images, not PS-X EXEs: `kf extract` slices
archive entries 2, 5, ..., 83 out of `CD\COM\FDAT.T`, and `kf init` checks
each by its own hash. `GAME.EXE` loads one at a time at `0x801E8308` (it keeps
that base, and `0x801E838C`, in its table at `0x800136E8`), so all 28 share one
window inside `GAME.EXE`'s address space. A module calls `GAME.EXE` functions
directly; those calls are cross-image references that stay unmodeled targets
until `GAME.EXE`'s owner is linked against the module. Each module starts
with its id (5 to `0x20`), 32 handler pointers, and the id plus `0x20` at
`0x84`; `GAME.EXE` calls the handlers through that table. The modules use
`GAME.EXE`'s `$gp` (`0x8009C214`). Linking a module is refused until the
module link is modeled.

Every authoritative address is keyed by `(image, va)`. One objdiff project
groups the independent object pairs under the image keys (`psx/`, `game/`,
`open/`, `end/`, `fdat_002/`, ...); equal addresses do not collide between
object pairs. A symbol, relocation, or function must never be joined across
images by address alone.

The repository still shares everything that should be shared:

- the pinned Psy-Q SDK and separate GNU/maspsx analysis route;
- platform headers, Sony library declarations, and reconstructed types;
- analysis scripts and Ghidra configuration;
- evidence and naming conventions; and
- source files proven byte-identical between programs.

The source tree is partitioned accordingly:

```text
src/
  psx/       PSX.EXE-only reconstruction
  game/      GAME.EXE-only reconstruction
  open/      OPEN.EXE-only reconstruction
  end/       END.EXE-only reconstruction
  fdat_nnn/  one FDAT module's reconstruction
  lib/       sources linked into more than one image (ADDRESS_AT claims)
```

This is therefore 32 decomps in one: 32 link graphs and match scores, with
one shared investigation and build environment.

## Address and naming rules

The structural inventory in `config/retail/functions.tsv` remains the source
of function boundaries. `functions_vendored.tsv` overlays Sony/Psy-Q provider
names without replacing that structure. The delinker selects names in this
order:

1. a structural function name;
2. a vendored-function name at the same `(image, va)`; or
3. a deterministic `func_<va>` placeholder.

Duplicate preferred names within one executable receive an address suffix.
They do not need suffixes merely because another executable uses the same name.

Data and relocation ownership will eventually be assigned to translation
units. That ownership is not inferable from the linked image alone, so the
initial pipeline deliberately carves one object per contiguous function. It is
a calibration topology, not a claim that the original source had one-function
files. Provider-identified Sony/Psy-Q functions never become reconstruction
or objdiff units; every function in the Psy-Q text bands has a
`functions_vendored.tsv` row (see
[the inventory](function-and-data-inventory.md#psy-q-library-text)).
Functions with multiple fragments and no fragment-range manifest are withheld
rather than guessed.

## Matching tiers

There are two different goals:

1. **Object match.** Compile reconstructed C or assembly, compare its MIPS ELF
   object to the carved target object, and iterate quickly with objdiff.
2. **Retail link match.** Reconstruct original Psy-Q object boundaries, library
   extraction, link order, linker script/options, overlay loading, and PS-X EXE
   packaging with the native tools.

The first tier can progress while the exact compiler and link topology remain
under investigation. A 100% function score proves the chosen object model and
code bytes for that comparison; it does not by itself prove the final Psy-Q
link or the historical translation-unit boundary.

See [delinking-and-matching.md](delinking-and-matching.md) for the implemented
first-tier workflow.
