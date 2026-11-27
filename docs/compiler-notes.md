# Compiler notes for matching

How Sony's GNU C 2.60 (`CC1PSX.EXE`, `-O2` with `-G8` or `-G0`), ASPSX 2.21
and DMPSX 2.03 turn this game's C into retail's instructions. The toolchain
and its evidence are in [toolchain.md](toolchain.md). Every rule below was
found on a function that did not match, and names a function where the
retail source form now stands: GAME unless another image is given; find it
with `grep -rn 'ADDRESS(0x...' src`. Where a cause names a GCC function or
file, it was read in the GCC 2.5.7 and 2.6.3 sources (no 2.6.0 source tree is
preserved) and checked against this compiler's output; a rule that rests only
on the shape of the output says so.

The rules explain residues; they do not excuse one, and
[AGENTS.md](../AGENTS.md) still decides what a source may contain. Fake
locals, dead or repeated statements, duplicated tails and gotos are accepted
devices once the semantics agree; `volatile` appears only where retail
evidence put it (see [volatile](#volatile)). Register names are the
assembler's (`v0`, `a0`, `s0`); a "pseudo" is GCC's virtual register before
allocation.

## Method

**Native compiles and RTL dumps.** `cc1psx-260` (the Decompals rebuild of
GCC 2.6.0, in the flake) has produced instruction-identical output to the
floppy's `CC1PSX.EXE` on every unit compared, runs natively in seconds, and
takes GCC's dump flags. Preprocess with `cpppsx-260` and the build's
definitions, compile with the build's `-quiet -g -O2 -G8` (or `-G0`), and add
`-dr -dj -ds -dL -df -dc -dS -dl -dg -dR -dJ -dd` for the expansion, jump,
cse, loop, flow, combine, sched1, local-alloc (`.lreg`: refs and live length
per pseudo), global-alloc/reload (`.greg`), sched2, jump2 and delay-slot
dumps. An include overlay for `cpppsx-260` must live on the WSL file system:
it cannot stat headers under `/mnt/c`.

**Authority.** Use native compiles to search; confirm with `kf try` and
`kf analyze`. See [verification traps](#verification-traps) for what a
native-only comparison hides.

**Pass order.** expansion, jump, cse, loop, jump again, cse2, flow, combine,
sched1, local-alloc, global-alloc (with reload), sched2, jump2, reorg. The
jump pass after loop (`toplev.c`) has no dump of its own; like the first it
may duplicate a loop's exit test (see [loop.c](#loopc)). Consequences used
throughout:

- flow counts references before combine, and nothing lowers the count
  afterwards, so uses that combine folds away still count for allocation.
- sched1 runs before local-alloc, so block-local births and deaths follow the
  scheduled order, not the source order.
- jump2 (the only pass that cross-jumps) runs after sched2, so both copies of
  a merged tail were present for the allocators and the schedulers.

## Toolchain facts

- **Compiler.** "GNU C 2.6.0 [AL 1.1, MM 40]" from Sony's 2.60 floppy. It
  calls `__main` at the top of `main` (supplied by `2MBYTE.OBJ`). Plain `char`
  is unsigned (`lbu`), as the CCPSX driver's `__CHAR_UNSIGNED__` says.
- **Profiles.** `-G8` and `-G0` occur per translation unit. All FDAT modules
  and `game.save_data` are `-G0` (no `$gp` use at all). A unit whose small
  globals are members of one aggregate compiles the same either way (the CD
  and sound state is one struct, reached absolutely): model the aggregate
  before choosing `-G0` for a unit.
- **ASPSX 2.21.** Expands cc1's macro instructions: `lw $2,sym` and `la` into
  `lui` plus a low half, `li` as `ori`, and `div` with its `break 7`/`break 6`
  checks (2.30 and 2.34 insert a hazard `nop` before the `div`; 2.05 and 2.08
  cannot address `$gp`). It emits `$gp`-relative accesses only for small data
  *defined in the same file*: an `.extern X,4` from another unit still gets
  `lui`/low. So a `$gp` access proves that the datum's owner is in the same
  translation unit, and an absolute access to a small datum means another TU
  (or `-G0`).
- **Volatile aggregates.** A member of a `volatile` aggregate is reached
  through `lui/addiu` and `0(reg)`, with the address kept in a register across
  a loop; a plain `volatile` scalar gives the `lw sym` macro form (the frame
  gate `0x80019614`).
- **DMPSX 2.03 and `INLINE.H`.** `#include <INLINE.H>` (plus `<GTEMAC.H>` for
  `gte_MulMatrix0`) is all a unit needs; the build runs DMPSX on any object
  with placeholder words. Each macro moves its operands into `t4`..`t7`
  (`$12`..`$15`) and emits a placeholder that DMPSX rewrites (`0x80035394`,
  `0x8003bb04`, the `game.render` front ends). The asm statements are
  volatile and flush CSE. Recognition:

  | Retail | Macro |
  | --- | --- |
  | `move t4,X; lwc2 $0,0(t4); lwc2 $1,4(t4)` | `gte_ldv0` (`ldv3`: also `$2/$3` via `t5`, `$4/$5` via `t6`) |
  | `mtc2 t4,$12; mtc2 t6,$14; mtc2 t5,$13` (values, not pointers) | `gte_ldsxy3` |
  | `nop; nop; cop2 0x180001` / `0x1400006` / `0x480012` / `0x486012` | `gte_rtps` / `gte_nclip` / `gte_rt` / `gte_rtv0` |
  | `swc2 $14` / `swc2 $24` / `swc2 $25,$26,$27` | `gte_stsxy` / `gte_stopz` / `gte_stlvnl` |
  | `mfc2 t5,$19; nop; sra t5,t5,2; sw` | `gte_stszotz` |
  | `ctc2` to `$0..$4`, `$5..$7`, `$8..$12`, `$16..$20` | `SetRotMatrix`, `SetTransMatrix`, `SetLightMatrix`, `SetColorMatrix` |
  | `sll 4` on three values, `ctc2 $13..$15` | `gte_SetBackColor` |
  | `lhu 0/6/12(t4); mtc2 $9..$11; rtir; mfc2; sh`, three times | `gte_MulMatrix0` (after `SetRotMatrix`) |

## Translation units and data layout

- **`.rdata` order.** Within one compilation cc1 emits local array
  initializers and string literals first, then the jump tables in function
  order. In PSX `main` the literals come out in reverse: the source's OPEN,
  GAME, END path table is laid out END, GAME, OPEN. Literals that follow a
  jump table therefore start a new TU (`0x80013680` in `game.movie`).
- **Initializer constants take the array's type.** Under `-G8`
  `char s[2] = " "` is `.sdata` while `char path[40] = "bu00:"` is `.rdata`,
  and a later identical literal shares that constant. Literals of at most 8
  bytes go to `.sdata`: their address is built with `lui/addiu`, their bytes
  are read `$gp`-relative.
- **Jump tables are `.align 3`** inside a compilation, so a 4-byte pad
  between two tables is that alignment and proves nothing; a table at 4 mod 8
  directly after another table's end cannot share its `.rdata` and starts a
  new TU (`0x8003260c` begins the TU after the player's).
- **`$gp` merges TUs.** `0x8003c35c`, `0x8003df50` and `0x800400ac` read
  `map_textured_primitive_color`, a datum of the hud code (`..0x8004290c`), as `$gp+0xcc`:
  screen, objects and hud are one TU (`game.render`).
- **Unreferenced statics and globals are emitted.** `RODATA`/`SDATA` claims
  are the object's real extent, without a trailing alignment pad.
- **Jump-table entries are `.text` offsets of the unit.** A non-exact
  function of the wrong size ahead of a table in the same unit moves every
  entry, and `kf verify data` reports `.rodata [addend]`; keep such a function
  in its own WIP slice until it matches (`game.floor_select`,
  `game.entity_sound`).

## Register allocation

### Priorities

**global-alloc** sorts pseudos by `floor_log2(refs) * refs / live_length`
(`allocno_compare` in `global.c`), compared as truncated integers (times
10000); equal values go to the lower pseudo number, which is declaration or
first-use order, and parameter order for parameters (reordering K&R parameter
declarations changes nothing).

**local-alloc** runs first and ranks block-local quantities by
`floor_log2(refs) * refs / ((death - birth) * size)`, births and deaths at two
per insn in sched1's order (`local-alloc.c`). A block-local pseudo that
crosses calls takes `s0`.. before any global pseudo is placed. A call does not
end a basic block in GCC 2.x, so a pointer used before and after a call inside
one branch is block-local when it is declared in that branch (FDAT_032
`0x801e8708`).

What decides the numbers:

- **Refs are flow's, before combine** (`combine.c` lowers only `reg_n_sets`).
  Statements that combine or CSE fold away still add references: a stored
  call result that is only compared (`dist = f(); if (dist != -1)`,
  `0x8004c1f0`), single-use pointer statements whose stores become absolute
  (`obj44 = &objects[121]; obj44->unk0 = 0;`, FDAT_020 `0x801e8a80`), each
  intermediate value of a door-pointer chain (FDAT_002 `0x801e87e0`).
- **Loop depth.** flow adds the loop depth per reference, so a use inside
  one loop counts 2 and a set in the preheader 1. A `retry:`/`goto` loop has
  no loop notes: no weighting, and loop.c leaves it alone (`0x800431e8`,
  where `count` loses to `scratch` only in the goto form).
- **Returns.** Each `return i` inside a loop is two references of `i`; one
  shared return can drop a variable below another (`0x80045c7c`).
- **Live length** follows statement order: reading `z` before `p` lengthens
  `z` (`0x800345f4`). A value set before a call crosses it (flow counts calls
  before sched1 moves the set), so `level = 0;` before the call gets a saved
  register and lands in the call's delay slot (`0x80026570`, `0x80026794`).
- **Stack-parameter equivalences.** `assign_parms` (`function.c`) gives a
  stack parameter's pseudo a `REG_EQUIV` to its incoming slot only while no
  conversion insns exist, and `update_equiv_regs` doubles the live length
  (halving the priority) of every pseudo with one. Any earlier narrow
  (`u8`/`u16`/`s16`) parameter adds a conversion and kills the equivalence
  of every later stack parameter: `s32 id` lets the fifth parameter `out` be
  doubled (`0x8004352c`), an `s32` third parameter lets the last stack
  parameter be read at its use (`0x80015410`). Such a parameter is read in
  place only with the equivalence and exactly two references.

- **Two symmetric chains in one block.** With `gx = ex - cx; gz = ez - cz;
  sx = -gx; sz = -gz; out.x = sx; out.z = sz;` sched1 keeps the x difference
  first and schedules its negation and stores after the whole z chain, so
  the x quantity has the long life: the z chain ranks first in local-alloc
  and takes `v0`, x takes `v1`, and the read-back of `sz` reuses `v0`. With
  the hard registers fixed like that, sched2 moves the x subtraction and its
  store below the z chain and leaves the z store next to its read-back.
  Writing the z pair first gives z the long life and `v1`, and sched2 then
  sinks the z store and read-back below the x stores (`0x80034bf4`).

### Hard-register choice

- MIPS has no `REG_ALLOC_ORDER`. `find_reg` pass 0 takes the call-used
  registers (`v0`..`t9`) in number order, skipping one that a
  lower-priority conflicting pseudo prefers; pass 1 takes the lowest free
  register and ignores preferences. Argument registers stay preferred by
  their parameters unless a hard-register conflict prunes the preference:
  retail's prologue loads `-40000` into `a2` while `z` (`a2`) is live, so `z`
  stops preferring `a2` and the walls pointer can take it (`0x8003260c`).
- A pseudo set from `(plus R const)` prefers `R`'s hard register
  (`set_preference`), so a loop giv based on a block-local pointer inherits
  its `s0` (FDAT_032 `0x801e8708`).

### Ties in local-alloc

local-alloc ties a dying operand to the result (one register, e.g.
`subu s0,s0,s3`) only when both are block-local (`combine_regs`): a
block-scoped `MapObject *made` dying in `made - objects` gives retail's tie in
`0x8005cbe0`, a function-scope one does not. The tie is also refused when the
result already has a quantity, which a hard-register operand earlier in the
insn gives it (`block_alloc` tries operands in order).

A hard-register operand has the same effect, and a returned local produces
one: with `return made;` the copy to `v0` has nothing to wait for, so sched1
moves it up behind the call that set `made`, and `optimize_reg_copy_1`
(`update_equiv_regs`) rewrites every later use of `made` as `v0`. In
`made - objects` operand 1 is then the hard register, the difference gets its
quantity from it and the dying base is not tied (`lui v1; addiu v1; subu
a0,v0,v1`, `0x8005c7e8`, whose callers ignore the returned record).

Quantities are allocated in priority order, so one more insn in a pointer's
life can move it behind three shorter-lived values: in `0x8004290c` the pool
pointer (3 references) gets `a2` after the three counters (2 each) only
because the `SCRATCH->model` load for `part` is scheduled inside its life.

### Source levers

- Declaration order breaks equal priorities (`0x80017c78`).
- Separate variables per case, phase or loop make separate allocnos; one
  shared variable conflicts everywhere (`0x80030fcc`, `0x8004e330`,
  `0x8003260c`, `0x800568bc`). One loop counter per loop (`0x8002c30c`).
- A loop index copied from a computed start (`cz = z0; ... cz++`) leaves the
  start block-local and the index a fresh global (`0x80033c4c`); a count kept
  apart from the loop index gets top priority (`0x8001aa84`, `0x8001d3a0`).
- `for (i = 0; i < n - 1; i++)` (no `n--`): loop.c hoists the bound and `n`
  becomes block-local, taking `s0` (`0x8001d0c4`, `0x8001cbb8`).
- Parameters shifted in place (`dir <<= 9`, `x >>= 11`, `radius *= 2`) keep
  their own saved register; a new expression makes a new pseudo
  (`0x8002d130`, `0x800443c8`, `0x80040708`).
- A constant held in a saved register is an initialised local (`last = 2`,
  `h = 240`, `part = 0xff`; `0x8001ec84`, `0x80035394`, `0x80030fcc`).
- One variable reused for two values is one pseudo: used in two blocks it is
  global, so local-alloc cannot tie it to its own extension (`u16 entry` for
  the archive entry and the page entry, `0x8001dbc4`), and if either use
  crosses a call both sit in a saved register (a glyph counter that later
  holds the cursor brightness counts in `s0`, `0x80025468`).
- A flag set directly before its loop has a shorter life, and so a higher
  priority, than one set at the top of the enclosing body (`end = 0;`,
  `0x80026acc`).
- A copy of a parameter declared before a block-move initializer is not
  rewritten back onto the parameter: the move's loop labels end cse's path
  between the copy and its uses (`u8 weapon = id;` before `u8 effect[34] =
  {...}`, `0x8002bdc0`).
- A pointer local initialised to a constant address that crosses calls and
  gets no saved register is not given a stack slot: reload rematerialises
  the constant at each use (`lui t8,0x1f80`, `0x80034bf4`; `li t2,255` for a
  hoisted constant, `0x80025468`).

## CSE

- **Constant addresses relate only through registers.** `explow.c` puts each
  constant address in a pseudo and cse rewrites later addresses of the same
  symbol as `reg + offset` (`use_related_value`). One pointer reassigned to
  each constant element gives retail's chains (`door = &objects[214]; ...
  door = &objects[111];` becomes `addiu v0,v0,-7004`; FDAT door handlers). An
  element index in a local (`k = 74; objects[k].unk38`) is folded to a
  constant only after expansion, so those accesses stay absolute `lui`/low
  (FDAT_026, FDAT_038; in a loop `0x80024a78`).
- **Members used twice share a base.** A non-volatile struct member loaded and
  stored (or two members of one global) goes through a CSE'd `lui/addiu`
  base; used once it is the `lw sym` form. `(base + n)->field` keeps the base
  register, which loop.c can hoist; `base[n].field` folds the offset into the
  constant (`0x8005f444`).
- **The address taken first becomes the base.** `u_long **buf =
  movie_state.vlc_buffers;` before `dec` makes `dec` `buf - 52` (`0x80061364`);
  `find_best_addr` prefers `(plus base off)` to a register at equal cost
  (`0x8002e3f8`).
- **No rehash on a register's first quantity.** `insert_regs` does not call
  `rehash_using_reg`, so `books + page` hashed while `page` had no quantity
  is never found again; a dead copy of `page` earlier in the block
  (`j = page;`) gives it one and lets cse2 reuse the value (`0x8001d944`).
- **Canonical register.** `make_regs_eqv` keeps the longer-lived register
  first, so after `mode = flags` the mask `mode = flags & 0xbf` is rewritten
  onto `mode`; a mode-changing copy (`u8 mode = flags`) is not an equivalence
  and the argument register is masked as in retail (`0x8003e34c`). After a
  jump equivalence (`record_jump_cond`) the compare's first operand stays
  canonical while it lives: one variable for the `write()` and the `read()`
  result is reassigned, leaves the class, and the second compare reuses the
  constant's register (`0x80028750`).
- **Invalidation.** Any store to a varying in-struct address forgets all
  struct memory, and so does a store to a fixed scalar; a store placed
  between two reads forces retail's reload (`0x800568bc`; `0x80053c84` case
  112 writes the `$gp` word first). A different spelling of the same value
  also defeats reuse: `U16(10) % 0x4000` reloads where `& 0x3fff` is shared
  with the later `& 0xc000` (`0x800500a8`).
- **Helper order.** King's Field II selects a part's vertices
  (`tmd_select_object_vertices`) before it takes the part pointer
  (`tmd_get_object`); with both inline, the two scratchpad stores come first
  and the part pointer is computed from a reloaded `scratch->model`
  (`0x8003e34c`, `0x8003f304`).
- **Followed jumps.** cse follows jumps into later blocks
  (`-fcse-follow-jumps` is on at `-O2`). An exit that jump1 cannot thread
  into a bare jump changes what cse2 sees: a dead store in the exit
  (`k = 0; break;`) keeps three area loads absolute in `0x8005cbe0`. `if/else`
  instead of `x = c; if (...) x = d;` keeps a value from being shared across
  the join (`0x8002c30c`), and a cross-jumped call starts a new extended
  block, so an address is recomputed after it.
- **Absolute addresses after a base.** cse1 rewrites a constant address as
  `base + offset` when a related address is already in a register on the
  path (`use_related_value`); cse2 folds it back to the constant when it
  still sees the base's value. A member updated on such a path (`x.m +=
  f()`) is then loaded and stored absolutely instead of through one address
  register. A statement after an `if` join starts a new path with an empty
  table, so the same update written once after the join keeps its address
  register; written in both arms it is absolute in each, and jump2 merges
  the two tails (`0x8002fe1c`). A pointer local set from a constant address
  (`sprite = &menu_sprite_defs[11]`) gives absolute member accesses the same
  way, with no base left for loop.c to hoist (`0x80025468`).
- **Extensions of different width are different expressions.** A `u8` field
  tested as an int is `(zero_extend:SI mem)`; the same field read in an
  expression assigned to a `u16` or `s16` is `(zero_extend:HI mem)`. cse
  does not relate the two, so the field is loaded twice, and reorg does not
  delete the second `lbu` as redundant because the patterns differ in mode
  (`se = r->sound_id + 144` with `u16 se`, `0x8002b928`, `0x8002ba38`). In
  the same way `id + 98` assigned to a `u16` re-extends `id` beside the
  `SImode` extension of the table index (`0x8002bdc0`).

## Expansion, fold and combine

- **Map cells.** `expand_expr` (both summands of a `PLUS`) always pushes a
  constant symbol outward, so `cells[z][x]` in any spelling becomes
  `z*800 + (x*10 + base)`. Retail's `(base + z*800) + x*10` is a block-scoped
  row pointer, fresh at each use site:

  ```c
  {
      MapCell *row = map_world.cells[obj->pos.vz >> 11];

      cell = &row[obj->pos.vx >> 11];
  }
  ```

  A function-scope `row` used at two sites is a global pseudo and gets other
  registers (`0x80044b40`, `0x80047010`).
- **Index forms.** `(&table[page * 9])[button + 1]` scales both parts
  separately, `[page * 9 + button + 1]` folds them (`0x80027688`);
  `&effects[x + 496]` distributes the constant into the base while
  `effects[x + 496].bank` adds first (`0x80040ae4`); `base->parts + i` gives
  `addu index,base`, `parts[i]` the reverse.
- **Reassociation.** fold moves constants out of a sum and combine puts them
  back on the first operand of the inner sum. Keep retail's order with the
  original grouping or a local: `-(t->vy + 1600) + at.vy` (`0x8004f1d0`),
  `vx - ((c << 11) + 1024)` (`0x80047010`), `dz += w - 1`, a local
  initialised with `h + 1600` (`0x80028d54`), `note + ((r >> 15) - 2)`
  (`0x8004ffa0`). For int-first `addiu; addu` with a pointer, use an integer
  temporary (`offset = part->vertices + 12; p = (u8 *)(offset + (s32)model);`).
- **`fold_truthop`.** Tests of adjacent fields merge into one word compare:
  `unke == 3 && unkf == 0xf0` is `lw; and; == 0xf0030000` (`0x8004b794`),
  `c8 != 0xff || c9 != 0xff` one `lhu != 0xffff`; nested `if`s prevent it.
  Consecutive constants in an `||` chain become range tests
  (`(u8)(x - k) < n`, `0x8004c668`); separate `if`s keep separate `beq`s.
  `pad & (A|B|C|D)` folds to one `andi`, four `||` terms keep four tests.
- **combine.** `x & (1 << n)` becomes `(x >> n) & 1` (`srav`); retail's
  `sllv; and` is `x & (bit = 1 << n)`. `x >= 0x8000` on a `u16`
  becomes `lbu` of the high byte and `srl 7`; `& 0x8000`, `>> 15` and
  `(s16)x < 0` do not (`0x80029500`, `0x80047010`).
- **Narrowing (`shorten` in `c-typeck.c`).** `obj->scale.vx >> 3` becomes a
  HImode shift whose extension cse folds (`lhu; sll; sra 19`), while
  `obj->scale.vx * 512 >> 12` keeps `lh; sra 3` (`0x800568bc`). `x % 30 == 0`
  on an `s16` tests `sll 16; bnez` (`0x80030fcc`). `(args[1] << 10)` narrows
  the load to `lhu`, `args[1] * 1024` keeps `lw` (`0x80053c84`). A `u8`
  dividend divides in QImode (`andi` on the quotient); a `u32` one gives
  `multu; srl`.
- **Division.** A constant divisor becomes a multiply; retail's real `div`
  needs a variable divisor (`n = 40; f = (x << 12) / n`, `0x80047010`).
  `x %= 4500` after `x / 4500` reuses the quotient.
- **Evaluation order.** Operand 0 of a compare is computed first
  (`0x800500a8`, `0x8004e330`); `[4] - [2]` loads `[4]` first.
- **Conditionals.** `return c ? p : 0` is branchless (`sltu; xori; negu;
  and`, `0x800194b0`); `cond ? 5 : 0` is `-(cond) & 5`; `r = 0; if (c) r = 1;
  return r;` keeps branches. A conditional assigned to its own operand
  (`a = a < 0 ? a + 0x1000 : a;`) is built in a temporary and copied back
  (`0x80015410`). `x = c == 0 ? 1 : 2;` stores later than per-arm stores,
  which cross-jump into one store (`0x800500a8`).
- **Chained assignment** stores the innermost target first (`a = b = c = 0`
  stores `c`, then `b`, then `a`); `s.vy = s.vz = s.vx += g` stores `vx`,
  `vz`, `vy` (`0x80056134`). A chained store shares one base for both members
  (`0x80035c44`).
- **Re-reads.** `if (++s.write == &s.requests[24])` reads once,
  `s.write++; if (s.write == ...)` twice. An increment written before an
  absolute store puts that store's address in a register
  (`stabilize_reference`, `0x8003da10`, `0x8003d8ec`).

## loop.c

- **Hoisting threshold.** `move_movables` starts at
  `2 * (1 + n_non_fixed_regs)` (52 with this compiler, 26 when the loop has a
  call) and lowers the threshold by 3 for every insn it moves; a candidate
  needs `threshold * savings * lifetime >= insn_count`. In a big loop the late
  invariants stay in place (`0x8003260c`'s `0x800 - q[k] + radius`). The
  lifetime is counted in insn positions including line-number notes, so two
  statements on separate lines are one further apart, and matching movables
  (the same constant loaded at several places) add their savings and
  lifetimes.
- **Moved once, counted double.** A register that an inner loop has already
  moved (`moved_once`) doubles `insn_count` for the rest of the outer loop's
  list, cumulatively, moved or not ("halved since already moved" in the `-dL`
  dump), and its lifetime is then the whole inner loop. A test of a `u8`
  parameter (`if (flags & 2)`) is three insns at loop time (the `and`, the
  truncation, the extension); written inside the inner loop of
  `0x800445b8` they are moved out of both loops, and the third doubling is
  what stops the outer loop from moving the second and third step of
  `col * 10`, exactly as in retail. The same test kept in a variable set
  before the loops gives another register allocation.
- **A user variable's giv is not reduced.** `strength_reduce` lowers the
  benefit of a giv whose register is a user variable, so
  `scale = &rec->scale;` (King's Field II has the same statement) stays an
  `addiu` in the loop while the argument expressions `&rec->pos` and
  `&rec->arg.m.animation_cache` become incremented registers (`0x80040ae4`).
- **Multi-set variables are not movable** (`scan_loop` wants one set): a
  variable assigned at several sites inside the loop is recomputed at each.
- **Where hoisted code lands.** An invariant spelled inside the loop goes to
  the preheader, after the loop's entry test; a pointer set before the loop
  is set where it is written (`0x8003c35c`'s `&scratch->w.model.unk54` in
  both loops, `0x8005c18c`'s `return p - 19 - script;`). In the preheader the
  hoisted constants come first and the giv initialisations after them
  (FDAT_032 `0x801e8708`); a user biv set before the loop precedes both
  (FDAT_020 `0x801e8a80`).
- **giv bases.** givs are prepended, so the base is the last address giv in
  source order (store `fog` before `z`, `0x8003e34c`).
- **Indexed loops are reversed.** `for (i = 0; i < 128; i++) objects[i]`
  counts down after loop.c; retail's count-down-plus-pointer loop is
  `for (o = ..., i = 128; i != 0; i--, o++)` (`0x80030fcc`). A block-scoped
  base indexed by an ascending `k` beside a descending counter is a reduced
  giv. A loop with multiplies in its body may be too large for strength
  reduction (`0x80023fd4`).
- **Loop-exit blocks.** `find_and_verify_loops` moves an exit block
  (conditional jump, block, jump out) behind the nearest barrier, searching
  backward first. Retail's `for (;;) { if (r != NONE) { f(); break; } ... }`
  then `return r;` after the loop (`0x80024c70`); in a void function `return;`
  inside works (`0x8001d944`). A `return value;` inside a `for (;;)` puts the
  return label before the loop end note, so its block never moves.
- **Exit-test duplication.** jump copies a rotated loop's exit test in front
  of the loop (`duplicate_loop_exit_test`) unless the test is over 20 insns
  or holds a call, a label or a block note. An indexed test
  (`a.m[e] >= table[e][n]`) is too long for the first jump pass; after
  loop.c has hoisted its addresses and reduced the index, the jump pass
  after loop duplicates it. By then loop.c may have moved an inner loop's
  exit block behind the entry jump's barrier, which keeps the copy's
  `if (c) goto body; goto end;` apart from the body until jump2 cross-jumps
  its compare into the bottom test: the operand loads appear before the
  entry jump (`0x80029f1c`).
- **Hoisting order.** Movables are taken in scan order, which starts at the
  exit test when the loop is entered at the bottom (threshold and doubling
  as above). Address invariants in the test can use up the budget before
  short-lived temporaries in the body are reached (`0x80029f1c`).
- **Rotation.** `expand_end_loop` (`stmt.c`) moves the exit test to the bottom
  unless it meets a call, a label or a block note. A test that stays at the
  top is `for (;;) { s32 n = *p++; if (n == 0xff) break; ... }`: the
  declaration's block note stops the rotation (`0x8005f444`);
  `while` and a plain `for (;;) { if (...) break;` rotate.

## Control flow

### Count loops

| Retail | Source |
| --- | --- |
| `beqz n` pre-test; `addiu i,-1; bne i,(-1)` | `for (i = n - 1; i != -1; i--)` (`n - 1 != -1` folds to `n != 0`) |
| `addiu; li -1; beq` pre-test, bottom `bne -1`, copy of `n` kept | `i = n; while (--i != -1)`, `n = *p++; while (--n != -1)` |
| `move v0,t; bnez v0; addiu t,-1` | `while (n--)` (testing the old value) |
| `li N; ...; addiu -1; bnez` | `for (i = N; i != 0; i--)` |
| `bgez` | `for (...; i >= 0; i--)` |
| `move; andi 0xffff; bnez; addu t,t,0xffff` | `u16` counter, `while (i--)` |
| `slti 4096; bnez; addiu` (increment in the slot) | `for (;;) { ...; if (i >= 4096) break; i += 128; }` |
| `slti N; bnez retry; addiu` (slot) | `retry: ... if (tries < N) { tries++; goto retry; }` |

The split `n--; } while (n != -1)` differs from `while (--n != -1)`
(`0x80033c4c`).

### Cross-jumping

jump2's `find_cross_jump` (`jump.c`) merges identical tails:

- An unconditional jump first tries the code falling into its label (one
  common insn suffices), then walks the label's jump chain, which is built in
  reverse insn order, and merges into the *first* jump with at least two
  common insns, not the longest. The minimum drops by one when the scan
  reaches a label, and when the first difference is a branch to the label
  right after the visiting jump (a jump around a jump).
- Jumps that cross-jumping creates to new labels are not merged again
  (`max_uid` guard). Retail's separate identical `li a0,N; j shared` blocks
  are cases that call the shared function themselves and `goto` past it
  (`case 6: player_probe_view_target_and_vectors(500, ...); goto spawn;`, `0x8002c30c`).
- **Which copy of `x = 0; break;` survives decides whether its label is
  new.** `if (c) { x = 0; break; }` has a branch before the store, so it
  merges (jump-around rule) only into a copy that also has a jump insn
  before it, never into a case that starts at a barrier. A copy reached
  through a label (`case 2: x = 0; break;`, or the store after an `&&`
  chain) merges into the first copy in the chain with one common insn.
  So when a plain `case 2: case 3: x = 0; break;` comes *before* the case
  that ends `if (c) goto use; x = 0; break;`, the plain block is merged into
  that case's tail at a label made during jump2, and the jumps that later
  reach it are never searched again: three identical
  `jal f; bnez use; j zero` tails stay separate. With the plain block last
  (KF2's fall-through order) the case's tail is merged into the old case
  label instead and the tails are merged on the next pass (`0x8004b984`).
- **A label before the store limits the merge to the store.** Written as one
  condition (`if (a && b && f()) goto use; x = 0; break;`), the false exits
  of `a` and `b` put a label before `x = 0`, the scan stops there, and only
  the store is merged; with `if (!a) { x = 0; break; } ... if (f()) goto use;
  x = 0; break;` the call and its test are merged too (`0x8004b984`).
- **The later copy survives.** Where retail keeps the earlier one, the source
  has an explicit `goto` into it (`0x800500a8` case 30, `0x800324f0`,
  `0x8005c1e8`, `0x80055db0`, `0x80053c84`'s jumps into a later case just past
  its own store). `mark_jump_label` canonicalises only consecutive labels, so
  plain source sometimes keeps an earlier copy too.
- Duplicated per-arm statements are how retail's shared tails arise: two
  calls of one function in `if/else` merge into one call with the argument set
  per arm (`beqz; move a0,zero; li a0,1; jal`, `0x8001a774`); per-arm stores
  merge into one store. A conditional expression or a shared variable does not
  reproduce that.

### Layout

- The then-block is laid first and falls through: `if (x != FREE) {...} else
  {...}` and its reverse differ.
- A block that retail places after the code and jumps back from is a `goto`
  label in source (`0x80017c78`'s `busy:`, `0x8001fa60`).
- `if (c) { x; break; }` targets move behind the nearest earlier barrier. A
  branch straight to the epilogue with `v0` untouched is an old-C valueless
  `return;` in an `int` function; `return 0;` emits a `j; move v0,zero` stub
  (OPEN `play_movie`).
- A range test wrapping the body with the failure return last
  (`if (v >= lo && hi >= v) { ...; return 1; } return 0;`) differs from two
  early returns (`0x80056580`, `0x80030b24`).

### Switches

- Case bodies are emitted in source order.
- The decision tree comes from `balance_case_nodes`: without a cost table
  (small values such as 1, 3, 4) three nodes split at the middle, and a dead
  `case N: break;` counts, moving the pivot without emitting a test
  (`0x8004e330`). `case 3: case 4:` gives a range test where separate bodies
  give two nodes; a range of cases is one node (`0x8002cfe0`).
- A jump table whose case 0 is the default still needs `case 0: break;`;
  without it the table is biased by the lowest case (`addiu -1`)
  (`0x80061940`, FDAT_008).
- A two-value byte dispatch (`beq 0x1f; beq 0x20; j end`) is a two-case
  `switch`, not `if`s (`0x8004c668`, `0x8005c18c`).
  `switch ((s16)loading) { case 0: return; case 1: break; default: return; }`
  reads the member once where two `if`s re-read it (`0x80018358`).
- A value used only after a switch but computed before it is assigned to a
  local before the `switch` (`0x80053310`).

## Scheduling and delay slots

- **Three independent updates.** For three independent `+=`, stores or
  negations of `x`, `y`, `z`, the second and third source statements come out
  swapped: write `x, z, y` to get retail's `x, y, z` and the reverse
  (`0x800568bc`, `0x80056268`, `0x800562f0`). King's Field II's `x, y, z`
  order in `0x8003be34` also puts the cell's elevation read before the `z`
  store, which lets sched1 schedule the flag's extension at the top of each
  arm; the flag's life is then short enough to be allocated before `x` and `z`.
- **Memory disambiguation.** A load of a struct or array member is not moved
  above a store through a struct pointer; a fixed-address scalar load is
  (`0x8001a34c`). Loads through a pointer local set once from a constant
  address can pass stores through another such local (`0x8005cbe0`
  case 128). Fixed member loads rise above stack stores only when written
  first (`0x8001a9dc`).
- **Births.** sched1 gives single-set register births launch priority, so a
  constant load lands right before its first consumer; memory insns win ties
  by potential hazard; otherwise the later insn is placed later. Equal
  chains therefore keep source order: a sum built from locals assigned in
  retail's order (`left = ...; x = ...; f(left + x, ...)`) schedules as
  retail does where one expression does not (`0x8001b5c4`), and each local
  also keeps its own constant from fold's reassociation. Only a
  pseudo with one set in the whole function is a birth (`birthing_insn_p`):
  a load into a variable that is also assigned elsewhere keeps its source
  position among other non-births and stays ahead of a birth that becomes
  ready with it. Retail's `lhu v1,14(s1); lhu a2,16(s1)` in `0x80044d9c` is
  one temporary used for two values of the definition, 50 lines apart.
- **Memory order in both schedulers.** A varying struct load (`pool->end`)
  stays behind every earlier fixed-address struct store and ahead of every
  later one; two fixed-address members of one base register and different
  offsets are independent, and so is a scalar access to a fixed address
  (`*(u32 *)&SCRATCH->w.model.rgb`). A load can therefore rise above
  scratchpad stores written before it (`0x8003e34c`), and "load A, then
  store B" in retail puts B's statement after A's (`0x8004290c`).
- **Fillers and no-op copies.** A load costs two cycles to its consumer and
  is held back one cycle directly in front of a store; the gap is filled by
  the ready insn that came latest in the incoming order. sched2 first deletes
  moves whose two sides are `rtx_equal_p`; a copy through a paradoxical
  subreg (`(set (reg:SI a3) (subreg:SI (reg:HI a3) 0))`, a narrow variable
  passed in the register it lives in) is not one, is scheduled as a filler
  and disappears only in jump2. A load-use `nop` in retail where this build
  has a real filler can be such a copy (`0x800533e8`, `0x8005c308`, open).
- **An odd update among shifts.** In a run of `p[n] <<= 3` updates sched1
  pipelines two loads ahead in `v0`/`v1`; four, six or eight updates keep
  source order and five come out as 1, 3, 2, 5, 4. A different update in
  the run (`p[3] &= 0xff9f`) joins that pipeline. Written last through a
  block-scoped temporary (`{ u16 t; ...; t = p[3] & 0xff9f; p[3] = t; }`)
  its load is scheduled first and held in a third register (`a0`) while the
  shifts keep their own order (the case blocks of `0x8003591c`).
- **Loop notes are barriers.** No insn is scheduled across a
  `NOTE_INSN_LOOP_BEG`/`END` inside a basic block (`sched_analyze_insn`;
  read in the source, no match rests on it yet).
- **Delay slots (reorg).** An assignment written after an if-block can fill
  the branch's slot from its target (`0x8001eb3c`); `speed = 0` inside the
  first arm, not before the `if`, lands in that branch's slot (`0x800500a8`).
  An `int` function with no return value (retail's implicit `int`) keeps
  `v0` live at the return, so no `v0` write fills the last slot; declaring
  it `void` changes that slot (`0x800290d8`). The first-declared initialised
  scalar's `move sN,zero` fills the first slot (OPEN `main`).
- **A redundant insn at the branch target.** When a branch goes to a shared
  `x = 0; j end` block and an earlier slot on the same path already holds
  `move x,zero`, reorg skips the redundant store, retargets the branch to
  `end`, and then fills the slot from the fall-through (`bnez v0,end; li
  v0,-257`). A branch that reaches `end` with `x` already zero but is not
  written through the shared block takes `move v0,x` from `end` instead. In
  `if (a) {...} else if (b && c) { flags &= ~F; } x = 0; break;` both tests
  of the else arm go through the shared block (`0x8004b984`, type 25).

## Stack frames

- **Aggregates get slots at declaration**, used or not (`expand_decl`), the
  first-declared lowest. Unexplained holes in retail frames have come from
  unused locals (`RECT`, `CdlLOC`, `SVECTOR`, `VECTOR`, `MATRIX`, byte
  arrays) and from unused reload slots (below). Such a local stays, with a
  comment such as `/* the frame keeps 8 unused bytes here */`, as in the
  King's Field II decomp and under AGENTS.md's fake-local rule; padding
  without a declared local is not acceptable. A struct expression evaluated
  twice also allocates an unused 8-byte slot (a local for it does not).
- **Address-taken scalars** get their slot when `&x` is first expanded, so
  their order follows first use; an inner block moves one later.
- **Block lifetimes** (`assign_stack_temp`, `preserve_temp_slots`,
  `pop_temp_slots` in `function.c`). A block's locals are freed when the block
  ends; a new slot takes an exact-size free slot first (newest first), then
  the smallest larger one. Passing `&v` as a *register* argument moves `v`
  (and every address-taken slot at its level) one level up, so it outlives
  its block; passing it as a stack argument, or copying `v`, does not. So
  per-case blocks must stay open as long as retail keeps their slots
  (`0x800568bc`, `0x80047010`), and a block after a switch reuses the case
  blocks' slots (`0x800500a8`).
- **Reload slots in pseudo-number order.** `reload` calls `alter_reg` for
  every unallocated pseudo in register-number order and gives a slot to any
  with references, including sign-extension temporaries that combine folded
  into `lh` (unused 8-byte slots). A block-scoped local is numbered when its
  block is expanded, so moving its declaration into the block moves its
  slot (`s32 scale;` in `0x8002d2a0`).
- **Variadic definitions.** A prologue that stores `a0`..`a3` above the frame
  and reads a parameter back with `lbu` from its home is a `...` definition
  with `va_start(args, id)` (`0x8002c30c`). Homing starts at the
  last named register: `sw a2,8(sp); sw a3,12(sp)` means three named
  parameters (`0x8004f414`).
- **Stack parameters.** One declared `int` but used as `u8` is read with
  `lbu` from its incoming slot; a `u8`-typed one is copied to a saved
  register (`0x80029188`).

## Types and declarations

- **Prototyped narrow parameters.** The caller masks or extends; the callee
  masks only where it needs the integer. A prototyped `s16` parameter
  forwarded to an `int` parameter is re-extended at each use.
- **K&R narrow parameters.** The callee converts at entry and callers pass
  raw ints; the header prototype must say `int` (`s32` is `long` here and
  fails "argument doesn't match prototype"). `assign_parms` emits the
  conversions after all plain parameter copies, so the prologue copies the
  wide parameters first (`0x80053c84`: `move s4,a3` before `move s5,a0`).
- **`int x_in` plus a narrowing local** (`void f(int a_in) { s16 a = a_in;`)
  is code-identical to the K&R form in most functions and replaces it
  wherever it compiles the same (`0x80045c7c`, `0x80019f14`, `0x8002a310`,
  and the variadic `0x80053c84`). Where it swaps registers, a prototyped
  narrow parameter takes the K&R path when the callers already pass that
  type (`0x80019e48`: `u16 file, u16 entry`); where retail re-narrows per
  use, an `int` parameter with `(s16)` at the calls (`0x8004e928`). No K&R
  definition is left.
- **Callers decide.** "Callee narrows, callers do not" is a K&R definition
  with an `int` prototype, or a prototyped `u16` whose callers pass a word
  (`0x80033f38`). Where one image narrows and another does not, the images saw
  different declarations: `FDAT_MODULE` gives FDAT units an unprototyped view
  (`include/game/entity.h`), and a function called with five and six
  arguments in one unit had no prototype (`0x80053c84`).
- **Return types.** The callee's declared return type changes which register
  an `and` with its result lands in (`u16` against `s32`); an `s16` local for
  an `s16` result keeps it apart from the next argument (`0x80030b24`). A
  `u32` result tested against a `u32` mask is the first operand of the `and`
  (`and v0,v0,s1`), a `u8` result the second (`map_cell_layer_mask`,
  `0x80040ae4`).
- **Extension patterns.** `lhu` then `sll/sra` = an `s16` local used raw and
  as an int (`s16 va = *a++`). `s16 d = (a - b) & 0xfff` masks before the
  extension. `s16` locals passed in a loop give a hoisted `sll` with the `sra`
  per call (`0x8005c0d4`). An `int` local for a `u8` field gives `slti`, the
  field itself `sltiu` (`0x8005eea8`). `srl` in a multiply chain means `u32`
  operands. A `u16` dividend promoted to int divides unsigned; copy it to an
  `int` local for `div`.
- **Struct copies are one block move** scheduled as a unit (`0x8004e824`) and
  use the type's alignment: `VECTOR` copies with `lw/sw`, `SVECTOR` with
  `lwl/lwr` even at aligned addresses. Retail's `lw/sw` copy of a `CVECTOR`
  is `*(u32 *)&dst = *(u32 *)&src`.
- **Inline block moves.** `memcpy` with a constant size is expanded inline
  (`memset` stays a call), with the alignment of each address's *base
  object*: `memcpy(s->b, ...)` is word-aligned when `S` has a word member, a
  `u8 *` or `u8[]` global gives the run-time aligned/unaligned loop pair, or
  `lwl/lwr` and `lb` for small sizes; casts to `(u32 *)` change nothing
  (`0x8005f7bc`). `strcpy(buf, "literal")` is an aligned block move of
  `strlen + 1` bytes, while `char name[] = "..."` stores with `swl/swr`
  (`0x80060d20`).

## volatile

The tree uses `volatile` in three places, each because retail shows it:

- the CD state's VSync counter `frame_gate` (`CD_VSYNC_SHARED` in
  `include/game/cd.h`), set by the VSync callback and polled by the frame gate
  `0x80019614`;
- `ResourceState.step` (`SCENE_STEP_SHARED` in `include/game/data.h`), set by
  the CD callbacks and volatile only in the scene, render and floor-select
  modules;
- the local wait counter `volatile u_long cnt` of the movie sample's decode
  wait in `game.movie` (`0x8006179c`), which retail keeps in its stack slot.

A volatile member makes the whole aggregate volatile for GCC
(`C_TYPE_FIELDS_VOLATILE`): every member access keeps program order, the
first access to a member's address uses `lui/addiu` and `0(reg)`, and `x++`
and `a = b = c` re-read. One volatile member reproduces all of the CD
module's accesses, while making the whole object volatile breaks some. Other
modules' accesses to the same objects (sound, image, menu) show none of it,
so the qualifier is defined by a macro only under the owning modules'
`GAME_*_MODULE`. The step's readers show it as a reload per compare (`lhu`
and `sll/sra` five times in `0x80030fcc`) and as `lui/addiu` plus `sh
0(reg)` stores (`0x800182e0`..). `0x8002b928` and `0x8002ba38` re-read a `u8`
record field, which a volatile pointer reproduces but which is a `u16`
local in the source (see [CSE](#cse)): a second load is not by itself
evidence of `volatile`.

## FDAT modules

Each module is one `-G0` translation unit (`src/fdat/fdat_nnn.c`) that reads
GAME's small data with `lui`/low pairs. Its `.rdata` comes first: the
`const FdatHeader` handler table at the origin, then the jump tables in
function order with a zero word where `.align 3` needs one; `.text` and
`.data` follow without padding. Bodies shared by several modules live in
`src/fdat/shared/*.inc` and are compiled into each. FDAT units see some GAME
functions without prototypes (`FDAT_MODULE`), because FDAT call sites pass
raw words where GAME narrows. The link and the archive frame are in
[executable-linking.md](executable-linking.md#fdat-modules).

## Verification traps

- **cc1psx accepts broken source silently** through `kf`: FDAT sources that
  named non-existent struct members compiled with no error to smaller code;
  an undeclared global compiled its tests away; a missing `LIBSND.H`
  prototype made a call unconditional; `x != NULL` without
  `<kf/lib/null.h>` compiled to garbage; a second definition of a function
  in one file is ignored in favour of the first. `kf try` shows no diagnostic
  for these. Check implicit-declaration *warnings* (a clang pass over the
  unit), not just errors, and compare objects after every header change.
- **Native comparisons hide data addresses.** A comparator that normalises
  data references showed `unk12018[1]` and `unk12018[0]` as equal; only
  `kf try`'s relocation listing showed the wrong field.
- **Header changes reach every caller.** Widening a prototype parameter
  (`u16` to `s32`) is safe only after checking that each caller compiles to
  the same code (`0x8004352c`, `0x8005c7e8`).
- **README after banking.** `kf bank` regenerates README's match-score block,
  which is committed with the baseline; `kf link --compare-only` rewrites the
  EXE block. Restore only the EXE block; never `git checkout README.md` after
  `kf bank`.

## Open mechanisms

Residues whose mechanism is known but whose source form is not:

- `0x800533e8`: retail spills a HImode parameter with `sw` and reloads it with
  `lhu`; no source form makes combine produce that pseudo. Both calls also
  schedule as if one more no-op copy were present than here.
- `0x800608ec`: retail stops hoisting after four invariants. With the
  threshold at 52 the record loop needs 41 to 43 insns at loop time; the
  plain source has 36 (each narrow temporary for a loaded byte adds one).
- `0x80040ae4`: reorg (`relax_delay_slots`) swaps the `0xe5` test with the
  jump to the default case unless `mostly_true_jump` is non-zero; retail does
  not swap, as if that branch left an inner loop (a `do { } while (0)` around
  the loop body gives retail's branch but moves two saved registers). The
  constant 1 of the three mark stores is hoisted here because the object loop
  has 467 insns at loop time and the limit is 468.
- `0x8005c308`: sched2 moves `li a3,0xfff` into a load-use gap that retail
  leaves alone; retail's argument copy for `a1` was not a plain no-op there.
- `0x8003591c`: retail keeps the byte insert of `tag.code &= ~2` in the
  register copy of the head word *and* the `srl 24; andi 0xfd` of the
  switch, with the `0x00ffffff` mask hoisted out of both loops and
  allocated after the part counter and pointer. Combine keeps both only
  when a later store of the byte dies after the switch value was combined;
  the three-store form that does this references the mask three times and
  the allocation order needs two.
