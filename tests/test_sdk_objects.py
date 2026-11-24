"""SDK object census and the SDK text-band vendored channel."""

from __future__ import annotations

import struct
import tempfile
import unittest
from pathlib import Path

from scripts.kf.retail import VENDORED_CONFIDENCE
from scripts.kf.sdk_object_census import evidence_rows, find_matches
from scripts.kf.seed_vendored_functions import (
    Function,
    ObjectSymbols,
    apply_band_rows,
    load_sdk_bands,
)

BASE = 0x80011000


def code(*words: int) -> bytes:
    return struct.pack(f"<{len(words)}I", *words)


def obj(data: bytes, relocated: tuple[int, ...] = (), xdefs=((0, "f"),)) -> ObjectSymbols:
    mask = bytearray(b"\xff" * len(data))
    for offset in relocated:  # a HI16-style patch: the low half is the linker's
        mask[offset:offset + 2] = b"\0\0"
    return ObjectSymbols(data, bytes(mask), len(relocated), tuple(xdefs))


class SdkObjectCensusTests(unittest.TestCase):
    def setUp(self) -> None:
        self.big = code(*range(0x24020001, 0x24020011))  # 64 bytes, li v0,n
        self.stub = code(0x240A00B0, 0x01400008, 0x24090017, 0)  # 16-byte B0 stub
        self.small = code(0x3C020000, 0x8C420000, 0x03E00008, 0)  # lui/lw, relocated
        image = (code(0) * 4 + self.big + self.stub
                 + code(0x3C028001, 0x8C421234, 0x03E00008, 0) + code(0) * 4)
        self.images = {"GAME.EXE": (BASE, image)}
        self.members = {
            ("LIBA.LIB", "BIG"): obj(self.big),
            ("LIBA.LIB", "STUB"): obj(self.stub, xdefs=((0, "SetMem"),)),
            ("LIBB.LIB", "SMALL"): obj(self.small, relocated=(0, 4), xdefs=((0, "g"),)),
            ("LIBA.LIB", "PART"): obj(self.big[:32]),  # inside BIG: dropped
        }

    def test_unique_matches_classify_by_size_and_starts(self) -> None:
        matches = find_matches(self.members, self.images)
        self.assertEqual([(m.va, m.members) for m in matches], [
            (BASE + 0x10, (("LIBA.LIB", "BIG"),)),
            (BASE + 0x50, (("LIBA.LIB", "STUB"),)),
            (BASE + 0x60, (("LIBB.LIB", "SMALL"),)),
        ])
        starts = {("GAME.EXE", BASE + 0x50), ("GAME.EXE", BASE + 0x60)}
        sections, complete, rejected = evidence_rows(matches, self.members, self.images, starts, 32)
        self.assertEqual([(row["module"], row["ram_va"], row["match_confidence"])
                          for row in sections],
                         [("BIG", f"0x{BASE + 0x10:08x}", "short_unique"),
                          ("SMALL", f"0x{BASE + 0x60:08x}", "short_unique")])
        self.assertEqual([(row["module"], row["name"]) for row in complete], [("STUB", "SetMem")])
        self.assertEqual(rejected, [])

    def test_small_match_off_any_start_is_rejected(self) -> None:
        matches = find_matches(self.members, self.images)
        _sections, complete, rejected = evidence_rows(matches, self.members, self.images, set(), 32)
        self.assertEqual(complete, [])
        self.assertEqual(len(rejected), 2)


class SdkBandTests(unittest.TestCase):
    def test_unidentified_band_functions_are_unmatched_version(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "bands.tsv"
            path.write_text("image\tstart\tend\tnote\nGAME.EXE\t0x80061be4\t0x80061c40\ttext\n")
            bands = load_sdk_bands(path)
        functions = [
            Function("GAME.EXE", 0x80061bc4, 0x20, "", ()),  # game code before the band
            Function("GAME.EXE", 0x80061be4, 0x10, "", ()),  # identified below
            Function("GAME.EXE", 0x80061bf4, 0x30, "", ()),  # unidentified
            Function("GAME.EXE", 0x80061c24, 0x20, "", ()),  # crosses the band end
        ]
        rows = {("GAME.EXE", 0x80061BE4): {
            "library": "LIBAPI.LIB", "module": "C159", "name": "SetMem", "va": "0x80061be4",
        }}
        apply_band_rows(functions, bands, rows, path)
        self.assertEqual(sorted(rows), [("GAME.EXE", 0x80061BE4), ("GAME.EXE", 0x80061BF4)])
        row = rows[("GAME.EXE", 0x80061BF4)]
        self.assertEqual((row["confidence"], row["evidence"], row["size"]),
                         ("sdk-unmatched-version", "sdk-text-band", "0x30"))
        self.assertIn("follows LIBAPI.LIB/C159 SetMem", row["note"])
        self.assertIsNotNone(VENDORED_CONFIDENCE.fullmatch(row["confidence"]))


if __name__ == "__main__":
    unittest.main()
