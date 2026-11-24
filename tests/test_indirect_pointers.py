"""Controls for reviewed run-time code pointers (indirect_pointers.tsv)."""

from __future__ import annotations

import struct
import tempfile
import unittest
from dataclasses import replace
from pathlib import Path

from scripts.kf.data_reachability import audit
from scripts.kf.indirect_pointers import (
    IndirectSite,
    PointerTarget,
    load_sites,
    resolve,
    source_images,
)
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.retail import OVERLAY_SOURCES
from scripts.kf.sema.evidence import Origin, Reference
from scripts.kf.sema.image import RetailImage
from scripts.kf.sema.index import Binding

IMAGE = "GAME.EXE"


def words(*values):
    return struct.pack(f"<{len(values)}I", *values)


def function(va, size=0x20):
    return Binding(IMAGE, va, size, "text", "function", f"fn_{va:x}", "", "test", "test",
                   body_size=size)


def write(path: Path, header: str, *rows: str) -> None:
    path.write_text("\n".join(("# test", header, *rows)) + "\n", encoding="utf-8")


class ResolveTest(unittest.TestCase):
    def config(self, pointer_rows, site_rows=None):
        directory = Path(tempfile.mkdtemp())
        write(directory / "functions.tsv", "image\tva", f"{IMAGE}\t0x1100", f"{IMAGE}\t0x1200")
        write(directory / "functions_vendored.tsv", "image\tva")
        write(directory / "relocs.tsv", "image\tsite_va\tkind\ttarget_va\tstatus",
              f"{IMAGE}\t0x2000\tmips32_candidate\t0x1100\treviewed",
              f"{IMAGE}\t0x2004\tmips32_candidate\t0x1200\tcandidate")
        write(directory / "indirect_pointers.tsv",
              "image\tpointer\tsource\tsource_image\tsource_va\tevidence", *pointer_rows)
        write(directory / "indirect_sites.tsv",
              "image\tsite_va\tfunction_va\tfunction\tpointer\tload_offset\tholder_va\tevidence",
              *(site_rows if site_rows is not None else
                (f"{IMAGE}\t0x1014\t0x1000\tfn\tHooks.unk40\t0x40\t\tthe only load",)))
        return directory

    def resolve(self, *rows, references=()):
        memory = {(IMAGE, 0x2000): 0x1100, (IMAGE, 0x2004): 0x1200}
        return resolve(IMAGE, references, config_dir=self.config(rows),
                       read_word=lambda image, va: memory.get((image, va)))

    def test_table_word_needs_a_reviewed_pointer_row_matching_the_bytes(self):
        targets, problems = self.resolve(
            f"{IMAGE}\tHooks.unk40\ttable-word\t{IMAGE}\t0x2000\tdefault table",
            f"{IMAGE}\tHooks.unk40\ttable-word\t{IMAGE}\t0x2004\tunreviewed word")
        self.assertEqual(targets, {"Hooks.unk40": (
            PointerTarget("Hooks.unk40", IMAGE, 0x1100, "table-word", IMAGE, 0x2000),)})
        self.assertEqual(len(problems), 1)
        self.assertIn("no reviewed pointer row", problems[0])

    def test_address_source_needs_a_validated_reviewed_reference_to_a_function(self):
        reviewed = (Origin("validated", "reachable-code", "manual", "reviewed", "test"),)
        refs = (Reference(IMAGE, 0x1050, 0x1200, "address", 0x1000, 0x1054, "validated",
                          reviewed),
                Reference(IMAGE, 0x1060, 0x1204, "address", 0x1000, 0x1064, "validated",
                          reviewed))
        targets, problems = self.resolve(
            f"{IMAGE}\tReq.done\taddress\t{IMAGE}\t0x1050\tpassed by caller",
            f"{IMAGE}\tReq.done\taddress\t{IMAGE}\t0x1060\tnot a function start",
            f"{IMAGE}\tReq.done\taddress\t{IMAGE}\t0x1070\tno reference", references=refs)
        self.assertEqual([t.va for t in targets["Req.done"]], [0x1200])
        self.assertEqual(len(problems), 2)

    def test_rows_without_evidence_are_problems(self):
        targets, problems = self.resolve(f"{IMAGE}\tHooks.unk40\ttable-word\t{IMAGE}\t0x2000\t ")
        self.assertEqual(targets, {})
        self.assertEqual(len(problems), 1)

    def test_sites_load_for_their_image_with_an_optional_holder(self):
        sites = load_sites(IMAGE, self.config((), (
            f"{IMAGE}\t0x1014\t0x1000\tfn\tHooks.unk40\t0x40\t\tthe only load",
            f"{IMAGE}\t0x1034\t0x1020\tfn\tHooks.unk44\t0x44\t0x100\tfrom the holder",
            "END.EXE\t0x1014\t0x1000\tfn\tHooks.unk40\t0x40\t\tanother image",
        )))
        self.assertEqual(sites, (
            IndirectSite(IMAGE, 0x1014, 0x1000, "Hooks.unk40", 0x40, "the only load"),
            IndirectSite(IMAGE, 0x1034, 0x1020, "Hooks.unk44", 0x44, "from the holder", 0x100),
        ))

    def test_overlay_wildcard_names_every_module_of_the_host(self):
        names = source_images(IMAGE, "FDAT_*.BIN")
        self.assertEqual(names, tuple(sorted(
            name for name, source in OVERLAY_SOURCES.items() if source.host == IMAGE)))
        self.assertEqual(len(names), 28)
        self.assertEqual(source_images(IMAGE, IMAGE), (IMAGE,))


class AuditTest(unittest.TestCase):
    # lui v0,0; lw v0,0x100(v0); nop; lw v0,LOAD(v0); nop; jalr v0; nop; jr ra; nop
    def report(self, load=0x40, *, site=0x1014, owner=0x1000, targets=None, holder=None,
               base=0x8C420100):
        code = words(0x3C020000, base, 0, 0x8C420000 | load, 0, 0x0040F809, 0,
                     0x03E00008, 0)
        img = RetailImage.synthetic(IMAGE, 0x1000, code + b"\0" * 0x200)
        caller = replace(function(0x1000), size=0x24, body_size=0x24)
        hook = PointerTarget("Hooks.unk40", IMAGE, 0x1100, "table-word", IMAGE, 0x1200)
        overlay = PointerTarget("Hooks.unk40", "FDAT_X.BIN", 0x801EB600, "table-word",
                                "FDAT_X.BIN", 0x801EB5A4)
        return audit(
            IMAGE, (caller, function(0x1100)), (),
            (Reference(IMAGE, 0x1014, None, "indirect-call", 0x1000, tier="proven"),), img,
            indirect_sites=(IndirectSite(IMAGE, site, owner, "Hooks.unk40", 0x40, "test",
                                         holder),),
            pointer_targets={"Hooks.unk40": (hook, overlay)} if targets is None else targets)

    def test_reviewed_pointer_site_reaches_every_target(self):
        report = self.report()
        self.assertEqual(report["issues"], [])
        self.assertEqual(report["pointer_dispatch"], [{
            "owner": "function:00001000", "site": 0x1014, "pattern": "reviewed-pointer",
            "pointer": "Hooks.unk40", "load_offset": 0x40, "holder": None,
            "targets": ["function:00001100", "FDAT_X.BIN:function:801eb600"],
        }])
        self.assertEqual(report["summary"]["reviewed_pointer_dispatch"], 1)
        self.assertIn("function:00001100", {r["key"] for r in report["ranges"]})

    def test_site_loading_another_member_stays_unresolved(self):
        report = self.report(load=0x44)
        self.assertEqual(report["summary"]["issues"], {"unresolved-indirect-control": 1})
        self.assertIn("does not load offset 0x40", report["issues"][0]["problem"])

    def test_holder_word_is_checked_through_the_base_load(self):
        self.assertEqual(self.report(holder=0x100)["issues"], [])
        report = self.report(holder=0x104)
        self.assertEqual(report["summary"]["issues"], {"unresolved-indirect-control": 1})
        self.assertIn("does not load the base from 0x00000104", report["issues"][0]["problem"])

    def test_holder_needs_a_loaded_base(self):
        report = self.report(holder=0x100, base=0x24420100)  # addiu v0,v0,0x100
        self.assertEqual(report["summary"]["issues"], {"unresolved-indirect-control": 1})

    def test_pointer_without_checked_sources_stays_unresolved(self):
        report = self.report(targets={})
        self.assertEqual(report["summary"]["issues"], {"unresolved-indirect-control": 1})

    def test_row_for_a_site_the_audit_never_meets_is_stale(self):
        report = self.report(site=0x1018)
        self.assertEqual(report["summary"]["issues"],
                         {"stale-indirect-site": 1, "unresolved-indirect-control": 1})

    def test_row_naming_another_owner_stays_unresolved(self):
        report = self.report(owner=0x1004)
        self.assertEqual(report["summary"]["issues"], {"unresolved-indirect-control": 1})
        self.assertIn("is not 0x00001004", report["issues"][0]["problem"])


class CuratedTablesTest(unittest.TestCase):
    def test_every_curated_site_names_a_pointer_with_sources(self):
        pointers = {line.split("\t")[1]
                    for line in (RETAIL_CONFIG / "indirect_pointers.tsv").read_text().splitlines()
                    if line and not line.startswith(("#", "image\t"))}
        for image in ("GAME.EXE", "OPEN.EXE", "END.EXE", "PSX.EXE"):
            for row in load_sites(image):
                self.assertIn(row.pointer, pointers, row)
                self.assertTrue(row.evidence.strip(), row)


if __name__ == "__main__":
    unittest.main()
