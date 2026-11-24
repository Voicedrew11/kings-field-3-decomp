"""Controls for recorded check exemptions (config/check_exemptions.tsv)."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest import mock

from scripts.kf import check_exemptions
from scripts.kf.check_exemptions import Exemption, load
from scripts.kf.data_match import Allocation, CommonRequest, SectionDiff, UnitDataDiff
from scripts.kf.data_match import _apply_common_exemption

HEADER = "image\tunit\tcheck\tsections\treason\n"
ROW = Exemption("END.EXE", "end.main", "common-placement", (".sbss", ".bss"), "no archive")


def datum(symbol, section, size=4, scope="global"):
    return SimpleNamespace(symbol=symbol, section_name=section, size=size, scope=scope, va=0)


def unit(*data):
    return SimpleNamespace(image="END.EXE", unit="end.main", data=data)


class LoadTest(unittest.TestCase):
    def read(self, text):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "x.tsv"
            path.write_text(text)
            return load.__wrapped__(path)

    def test_rows_parse(self):
        rows = self.read("# c\n" + HEADER
                         + "END.EXE\tend.main\tcommon-placement\t.sbss,.bss\tno archive\n")
        self.assertEqual(rows, (ROW,))

    def test_unknown_check_or_section_is_rejected(self):
        with self.assertRaises(ValueError):
            self.read(HEADER + "END.EXE\tend.main\tdata-match\t.bss\tx\n")
        with self.assertRaises(ValueError):
            self.read(HEADER + "END.EXE\tend.main\tcommon-placement\t.data\tx\n")

    def test_missing_reason_is_rejected(self):
        with self.assertRaises(ValueError):
            self.read(HEADER + "END.EXE\tend.main\tcommon-placement\t.bss\t\n")


class DataExemptionTest(unittest.TestCase):
    def apply(self, owner, recon, retail=None, diffs=()):
        result = UnitDataDiff("END.EXE", "end.main", list(diffs))
        retail = retail or SimpleNamespace(allocations={})
        with mock.patch.object(check_exemptions, "load", return_value=(ROW,)):
            _apply_common_exemption(owner, retail, recon, result)
        return result

    def test_common_requests_of_the_claimed_size_are_exempt_not_matched(self):
        owner = unit(datum("cdb", ".sbss"))
        recon = SimpleNamespace(common_requests=[CommonRequest("cdb", 4, 4)], allocations={})
        result = self.apply(owner, recon,
                            diffs=[SectionDiff(".sbss", 4, 0, "missing")])
        self.assertTrue(result.passes)
        self.assertFalse(result.matches)
        self.assertEqual([d.status for d in result.diffs], ["exempt"])

    def test_a_static_in_a_covered_section_voids_the_exemption(self):
        owner = unit(datum("cdb", ".sbss"))
        recon = SimpleNamespace(common_requests=[CommonRequest("cdb", 4, 4)],
                                allocations={".sbss": [Allocation("frame", 0, 4, 0, 0)]})
        result = self.apply(owner, recon,
                            diffs=[SectionDiff(".sbss", 4, 4, "layout")])
        self.assertFalse(result.passes)
        self.assertIn("exemption-invalid", [d.status for d in result.diffs])

    def test_a_wrong_common_size_voids_the_exemption(self):
        owner = unit(datum("db", ".bss", size=0x8e8))
        recon = SimpleNamespace(common_requests=[CommonRequest("db", 0x400, 4)], allocations={})
        result = self.apply(owner, recon, diffs=[SectionDiff(".bss", 0x8e8, 0, "missing")])
        self.assertFalse(result.passes)


if __name__ == "__main__":
    unittest.main()
