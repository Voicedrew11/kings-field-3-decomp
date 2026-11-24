"""Recorded per-unit exemptions from checks that cannot run (config/check_exemptions.tsv).

An exemption never turns a failure into a pass silently. Each row names the
image, the unit, the check it skips, the sections it covers and the reason;
every report that applies it prints it as EXEMPT, and the README's exact-data
count does not count an exempt unit.

Only one check can be exempted:

``common-placement``
    The unit's global ``.sbss``/``.bss`` claims in the listed sections are
    COMMON requests that PSYLINK places by symbol-hash bucket among the SDK's
    own COMMON. Where the library objects that share those buckets are not
    preserved, no link can put them at their retail addresses, so their
    placement (the data gate's section comparison and the target relink's
    section bases) is skipped. Still checked: every claim in the listed
    sections must be such a COMMON request in the reconstruction, with the
    claimed size; a fixed (static) allocation there voids the exemption; all
    other sections, relocations and functions are compared as usual.
"""

from __future__ import annotations

from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path

from scripts.kf.paths import REPO

EXEMPTIONS = REPO / "config" / "check_exemptions.tsv"
CHECKS = frozenset({"common-placement"})
COMMON_SECTIONS = (".sbss", ".bss")


@dataclass(frozen=True)
class Exemption:
    image: str
    unit: str
    check: str
    sections: tuple[str, ...]
    reason: str

    def describe(self) -> str:
        return (f"EXEMPT {self.unit} [{self.check}: {', '.join(self.sections)}] "
                f"{self.reason}")


@lru_cache(maxsize=None)
def load(path: Path = EXEMPTIONS) -> tuple[Exemption, ...]:
    if not path.is_file():
        return ()
    rows = []
    header = None
    for number, line in enumerate(path.read_text().splitlines(), 1):
        if not line.strip() or line.startswith("#"):
            continue
        fields = line.split("\t")
        if header is None:
            header = fields
            if header != ["image", "unit", "check", "sections", "reason"]:
                raise ValueError(f"{path}:{number}: unexpected header {header}")
            continue
        if len(fields) != len(header) or not all(fields):
            raise ValueError(f"{path}:{number}: every column is required")
        image, unit, check, sections, reason = fields
        if check not in CHECKS:
            raise ValueError(f"{path}:{number}: unknown check {check!r}")
        names = tuple(s.strip() for s in sections.split(","))
        if not names or any(s not in COMMON_SECTIONS for s in names):
            raise ValueError(f"{path}:{number}: {check} covers only {COMMON_SECTIONS}")
        rows.append(Exemption(image, unit, check, names, reason))
    keys = [(row.image, row.unit, row.check) for row in rows]
    if len(set(keys)) != len(keys):
        raise ValueError(f"{path}: duplicate exemption rows")
    return tuple(rows)


def for_unit(image: str, unit: str, check: str = "common-placement") -> Exemption | None:
    return next((row for row in load() if (row.image, row.unit, row.check)
                 == (image, unit, check)), None)


def common_symbols(unit) -> dict[str, object]:
    """Global data claims of ``unit`` in its common-placement sections, by symbol."""
    exemption = for_unit(unit.image, unit.unit)
    if exemption is None:
        return {}
    return {datum.symbol: datum for datum in unit.data
            if datum.section_name in exemption.sections and datum.scope == "global"}
