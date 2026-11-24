"""Find pinned SDK objects whose relocation-masked .text occurs in a retail image.

Every member of every archive (and standalone OBJ) in ``--sdk-lib-dir`` is
listed with ``psy-k``; its joined ``.text`` is compared, with the bits each
linker patch owns wildcarded (the same mask ``kf-vendored-seed`` re-checks),
against the load area of each PS-X EXE. A member is kept only where it occurs
exactly once in an image, and a match lying wholly inside a larger match is
dropped. The output is the two evidence tables ``kf-vendored-seed`` verifies
and consumes:

* ``--sections-output``: matches of at least ``--minimum-bytes`` (default 32),
  and smaller relocated ones with an XDEF on an admitted function start,
  graded ``strong_unique`` (at least 1024 compared bits), ``short_unique`` or
  ``archive_ambiguous`` (several members with the same bytes);
* ``--complete-output``: smaller members whose unrelocated bytes equal the
  image bytes outright at an admitted function start, named by their
  offset-zero XDEF.

Anything else (a small match with no XDEF on a start, or one overlapping
another match) is printed as rejected.

Both go below ``build`` by default; review them before copying them to
``config/evidence``.
"""

from __future__ import annotations

import argparse
import os
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence

from scripts.kf.retail import IMAGE_LAYOUTS, IMAGE_ORDER, parse_int, read_tsv, write_tsv
from scripts.kf.seed_vendored_functions import (
    COMPLETE_OBJECT_FIELDS,
    OBJECT_EVIDENCE_FIELDS,
    ObjectSymbols,
    find_masked_bytes,
    load_retail_payloads,
    parse_object_symbols,
    resolve_tool,
    run_psyk,
)

STRONG_BITS = 1024


@dataclass(frozen=True)
class Match:
    image: str
    va: int
    size: int
    members: tuple[tuple[str, str], ...]

    @property
    def end(self) -> int:
        return self.va + self.size


def load_members(psyk: Path, sdk_lib_dir: Path) -> dict[tuple[str, str], ObjectSymbols]:
    """Every member with a .text section, keyed (ARCHIVE.LIB, MODULE)."""
    result: dict[tuple[str, str], ObjectSymbols] = {}

    def add(key: tuple[str, str], path: Path) -> None:
        try:
            result[key] = parse_object_symbols(run_psyk(psyk, "list", "--code", str(path)))
        except ValueError:
            pass  # no single .text section: nothing to place

    for archive in sorted(sdk_lib_dir.iterdir(), key=lambda path: path.name.upper()):
        suffix = archive.suffix.upper()
        if suffix == ".OBJ":
            add((archive.name.upper(), archive.stem.upper()), archive)
        elif suffix == ".LIB":
            with tempfile.TemporaryDirectory(prefix="kf-sdk-objects-") as directory:
                root = Path(directory)
                run_psyk(psyk, "extract", str(archive.resolve()), cwd=root)
                for obj in sorted(root.glob("*.OBJ")):
                    add((archive.name.upper(), obj.stem.upper()), obj)
    return result


def find_matches(
    members: dict[tuple[str, str], ObjectSymbols],
    images: dict[str, tuple[int, bytes]],
) -> list[Match]:
    """Unique aligned occurrences, grouped by location and size."""
    grouped: dict[tuple[str, int, int], list[tuple[str, str]]] = {}
    for key, obj in members.items():
        if obj.text_size < 8:
            continue
        for image, (load_address, payload) in images.items():
            try:
                offsets = find_masked_bytes(obj.text_data, obj.text_mask, payload)
            except ValueError:
                break  # no eight-byte exact anchor
            if len(offsets) == 1 and offsets[0] % 4 == 0:
                grouped.setdefault(
                    (image, load_address + offsets[0], obj.text_size), []
                ).append(key)
    matches = [
        Match(image, va, size, tuple(sorted(keys)))
        for (image, va, size), keys in grouped.items()
    ]
    # Drop a match lying wholly inside a larger one in the same image.
    kept = []
    for match in matches:
        if not any(
            other is not match and other.image == match.image
            and other.va <= match.va and match.end <= other.end
            and other.size > match.size
            for other in matches
        ):
            kept.append(match)
    return sorted(kept, key=lambda m: (IMAGE_ORDER[m.image], m.va))


def evidence_rows(
    matches: Sequence[Match],
    members: dict[tuple[str, str], ObjectSymbols],
    images: dict[str, tuple[int, bytes]],
    starts: set[tuple[str, int]],
    minimum_bytes: int,
) -> tuple[list[dict[str, object]], list[dict[str, object]], list[str]]:
    sections: list[dict[str, object]] = []
    complete: list[dict[str, object]] = []
    rejected: list[str] = []
    for index, match in enumerate(matches):
        overlapping = [
            other for other in matches
            if other is not match and other.image == match.image
            and other.va < match.end and match.va < other.end
        ]
        if overlapping:
            rejected.append(
                f"{match.image} {match.va:#x} {match.size:#x} {match.members}: "
                f"overlaps {[(hex(o.va), o.members) for o in overlapping]}"
            )
            continue
        candidates = ",".join(f"{library}/{module}/.text" for library, module in match.members)
        load_address, payload = images[match.image]
        offset = match.va - load_address
        first = members[match.members[0]]
        names = [name for position, name in first.xdefs if position == 0]
        whole = (len(match.members) == 1 and (match.image, match.va) in starts and names
                 and payload[offset:offset + first.text_size] == first.text_data)
        anchored = any((match.image, match.va + position) in starts
                       for key in match.members for position, _name in members[key].xdefs)
        if match.size >= minimum_bytes or (not whole and anchored):
            for library, module in match.members:
                obj = members[(library, module)]
                if len(match.members) > 1:
                    confidence = "archive_ambiguous"
                elif obj.compared_bits >= STRONG_BITS:
                    confidence = "strong_unique"
                else:
                    confidence = "short_unique"
                sections.append({
                    "library": library,
                    "module": module,
                    "block_size": obj.text_size,
                    "compared_bits": obj.compared_bits,
                    "relocations_masked": obj.relocation_count,
                    "image": match.image,
                    "image_occurrences": 1,
                    "archive_candidate_count": len(match.members),
                    "archive_candidates": candidates,
                    "match_confidence": confidence,
                    "ram_va": f"0x{match.va:08x}",
                })
            continue
        if not whole:
            rejected.append(
                f"{match.image} {match.va:#x} {match.size:#x} {match.members}: "
                "small match has no XDEF on an admitted start"
            )
            continue
        library, module = match.members[0]
        obj = first
        complete.append({
            "image": match.image,
            "va": f"0x{match.va:08x}",
            "object_size": f"0x{obj.text_size:x}",
            "library": library,
            "module": module,
            "name": names[0],
            "note": f"complete {obj.text_size}-byte .text equals the image bytes "
                    f"({obj.relocation_count} relocations)",
        })
    return sections, complete, rejected


def parse_args(arguments: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--psyk", type=Path)
    parser.add_argument(
        "--sdk-lib-dir", type=Path,
        default=Path(os.environ["PSYQ_LIB"]) if "PSYQ_LIB" in os.environ else None,
    )
    parser.add_argument("--sdk-label", default="the pinned Runtime Library 3.3 CD (1995-10-20)")
    parser.add_argument("--exe-dir", type=Path, required=True)
    parser.add_argument("--functions", type=Path, default=Path("config/retail/functions.tsv"))
    parser.add_argument("--minimum-bytes", type=int, default=32)
    parser.add_argument(
        "--sections-output", type=Path,
        default=Path("build/sdk-objects/psyq_33_text_sections.tsv"),
    )
    parser.add_argument(
        "--complete-output", type=Path,
        default=Path("build/sdk-objects/psyq_33_complete_objects.tsv"),
    )
    return parser.parse_args(arguments)


def main(arguments: Sequence[str] | None = None) -> int:
    args = parse_args(arguments)
    if args.sdk_lib_dir is None:
        raise SystemExit("Psy-Q library directory missing; enter nix develop or pass --sdk-lib-dir")
    members = load_members(resolve_tool(args.psyk, "psyk"), args.sdk_lib_dir)
    payloads = load_retail_payloads(args.exe_dir)
    images = {name: payloads[name] for name, layout in IMAGE_LAYOUTS.items()
              if layout.header_size}
    _fields, functions = read_tsv(args.functions)
    starts = {(row["image"], parse_int(row["va"])) for row in functions}
    sections, complete, rejected = evidence_rows(
        find_matches(members, images), members, images, starts, args.minimum_bytes
    )
    write_tsv(args.sections_output, OBJECT_EVIDENCE_FIELDS, sections, (
        f"Relocation-masked exact .text matches against objects from {args.sdk_label}.",
        f"Generated by scripts/kf/sdk_object_census.py --minimum-bytes {args.minimum_bytes}; "
        "unique executable occurrences only.",
    ))
    write_tsv(args.complete_output, COMPLETE_OBJECT_FIELDS, complete, (
        "Complete code-section matches too small for the section matcher "
        "(no relocated bytes differ).",
        "Generated by scripts/kf/sdk_object_census.py; each object sits at an admitted start.",
    ))
    for line in rejected:
        print(f"rejected: {line}")
    print(f"{len(members)} SDK members; {len(sections)} section rows, "
          f"{len(complete)} complete-object rows, {len(rejected)} rejected")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
