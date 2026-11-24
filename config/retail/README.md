# Retail executable censuses

These TSVs are the admitted, manually curated work-in-progress model of the
King's Field III `SLUS-00255` executables listed in `../images.toml`:

- `functions.tsv`: current function starts and explicit WIP extents;
- `functions_vendored.tsv`: provider/library ownership layered over those
  structural functions;
- `data.tsv`: defined data plus `unclassified` gaps;
- `relocs.tsv`: reconstructed relocation candidates and their review status;
- `function_identities.tsv`: source-level names, owners, actions, return types,
  and named parameters for all carveable non-vendored functions;
- `data_identities.tsv`: source-level names, extents, storage, linkage scope,
  types, and owners for initialized globals/statics and referenced BSS;
- `data_sections.tsv`: reviewed initialized section assignments;
- `structures.tsv` and `structure_fields.tsv`: target structure layouts with
  their evidence.

Each image is a separate linked program or overlay, so every row carries an
`image`. `GAME.EXE`, `OPEN.EXE` and `END.EXE` reuse the 0x80011000 load
region, and the 28 `FDAT_nnn.BIN` modules all load at 0x801E8308 inside
`GAME.EXE`'s address space: never merge rows on address alone. Overlay rows use file offsets
relative to the overlay image (it has no PS-X EXE header).
`functions.tsv`/`data.tsv` hold the reviewed `kf-function-audit` /
`kf-function-propose` census, `functions_vendored.tsv` the reviewed
`kf-vendored-seed` output (its SDK evidence is in `../evidence/`), and the
identity tables the reviewed `kf inventory` seeds; generators write below
`build/` and rows are admitted by review. See
`docs/function-and-data-inventory.md`.
