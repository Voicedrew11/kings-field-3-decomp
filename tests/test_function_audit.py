from __future__ import annotations

import struct
import tempfile
import unittest
from pathlib import Path

from scripts.kf.audit_functions import (
    FunctionExtent,
    ImageWords,
    audit_image,
    fallthrough_boundary,
    hoisted_start,
    matching_return,
    frame_at,
    switch_extension,
    transfer_end,
)
from scripts.kf.propose_function_admission import proposed_data, proposed_functions
from scripts.kf.retail import (
    DATA_FIELDS,
    FUNCTION_FIELDS,
    ImageLayout,
    overlay_handlers,
    overlay_header_end,
    write_tsv,
)


BASE = 0x80010000
JR_RA = 0x03E00008
NOP = 0


def words(*values: int) -> ImageWords:
    payload = struct.pack(f"<{len(values)}I", *values)
    image = ImageWords.__new__(ImageWords)
    image.layout = ImageLayout("T.EXE", "-", 0x800 + len(payload), BASE, BASE, len(payload))
    image.payload = payload
    return image


def jal(target: int) -> int:
    return 0x0C000000 | ((target >> 2) & 0x03FFFFFF)


def jump(target: int) -> int:
    return 0x08000000 | ((target >> 2) & 0x03FFFFFF)


class FunctionAuditTests(unittest.TestCase):
    def test_gcc27_epilogue_restores_the_stack_before_the_return(self) -> None:
        code = words(
            0x27BDFFE8,  # addiu sp,sp,-24
            0xAFBF0010,  # sw ra,16(sp)
            0x8FBF0010,  # lw ra,16(sp)
            0x27BD0018,  # addiu sp,sp,24
            JR_RA, NOP,
        )
        frame = frame_at(code, BASE)
        self.assertEqual(matching_return(code, BASE, frame), BASE + 0x10)

    def test_delay_slot_epilogue_still_matches(self) -> None:
        code = words(
            0x27BDFFE8,  # addiu sp,sp,-24
            0xAFBF0010,  # sw ra,16(sp)
            0x8FBF0010,  # lw ra,16(sp)
            NOP,
            JR_RA,
            0x27BD0018,  # addiu sp,sp,24 (delay slot)
        )
        self.assertEqual(matching_return(code, BASE, frame_at(code, BASE)), BASE + 0x10)

    def test_aspsx_division_checks_and_ori_constants_stay_inside_one_function(self) -> None:
        # ASPSX expands `div` with divide-by-zero and overflow traps and
        # spells `li` as `ori`; neither `break` nor the trap branches may end
        # the frame, and the leaf after the return is still carved.
        code = words(
            0x27BDFFE8,  # addiu sp,sp,-24
            0xAFBF0010,  # sw ra,16(sp)
            0x34050064,  # ori a1,zero,100   (li)
            0x0085001A,  # div zero,a0,a1
            0x14A00002,  # bnez a1,+2
            NOP,
            0x0007000D,  # break 7
            0x2401FFFF,  # li at,-1
            0x14A10004,  # bne a1,at,+4
            0x3C018000,  # lui at,0x8000
            0x14810002,  # bne a0,at,+2
            NOP,
            0x0006000D,  # break 6
            0x00001012,  # mflo v0
            0x8FBF0010,  # lw ra,16(sp)
            0x27BD0018,  # addiu sp,sp,24
            JR_RA, NOP,
            0x34020007,  # leaf: ori v0,zero,7
            JR_RA, NOP,
        )
        frame = frame_at(code, BASE)
        self.assertEqual(matching_return(code, BASE, frame), BASE + 0x40)
        rows = audit_image("T.EXE", code, [], {})
        starts = {row["va"]: row["confidence"] for row in rows}
        self.assertEqual(starts, {f"0x{BASE:08x}": "strong",
                                  f"0x{BASE + 0x48:08x}": "strong-leaf"})

    def test_called_division_hoisted_above_the_frame_extends_the_start(self) -> None:
        # GAME.EXE 0x80067c10 shape: a jal target, then a signed division with
        # its ASPSX traps scheduled above `addiu sp`, then the admitted frame.
        division = (
            0x00C2001A,  # div zero,a2,v0
            0x14400002, NOP, 0x0007000D,  # bnez v0,+2; nop; break 7
            0x2401FFFF,  # li at,-1
            0x14410004,  # bne v0,at,+4
            0x3C018000,  # lui at,0x8000
            0x14C10002, NOP, 0x0006000D,  # bne a2,at,+2; nop; break 6
            0x00003012,  # mflo a2
        )
        code = words(
            0x27BDFFE8, 0xAFBF0010, jal(BASE + 0x20), NOP,  # caller
            0x8FBF0010, 0x27BD0018, JR_RA, NOP,
            0x00042400,  # sll a0,a0,16 (callee starts here)
            *division,
            0x3402007F,  # ori v0,zero,0x7f
            0x27BDFFE8, 0xAFBF0010,  # admitted frame
            0x8FBF0010, 0x27BD0018, JR_RA, NOP,
        )
        frame = BASE + 0x20 + 4 * 13
        extents = [FunctionExtent(BASE, BASE + 0x20), FunctionExtent(frame, frame + 0x18)]
        self.assertEqual(hoisted_start(code, frame), BASE + 0x20)
        rows = audit_image("T.EXE", code, extents, {})
        extend = [row for row in rows if row["confidence"] == "call-extend"]
        self.assertEqual([(row["va"], row["owner_va"]) for row in extend],
                         [(f"0x{BASE + 0x20:08x}", f"0x{frame:08x}")])

    def test_branch_out_of_a_run_is_not_a_division_check(self) -> None:
        code = words(JR_RA, NOP, 0x14400003, NOP, 0x0007000D, NOP, 0x27BDFFE8, 0xAFBF0010)
        # bnez skips past the break (target is not right after it).
        self.assertIsNone(hoisted_start(code, BASE + 0x18))

    def test_loads_hoisted_above_the_frame_belong_to_the_function(self) -> None:
        code = words(
            JR_RA, NOP,
            0x3C028019,  # lui v0,0x8019
            0x8C450F1C,  # lw a1,3868(v0)
            0x27BDFFD8,  # addiu sp,sp,-40
            0xAFBF0020,  # sw ra,32(sp)
        )
        self.assertEqual(hoisted_start(code, BASE + 0x10), BASE + 8)
        branch = words(JR_RA, NOP, 0x10400002, NOP, 0x27BDFFD8, 0xAFBF0020)
        self.assertIsNone(hoisted_start(branch, BASE + 0x10))

    def test_call_target_after_a_stub_splits_its_owner(self) -> None:
        code = words(
            0x240A00B0, 0x01400008, 0x24090032, NOP,  # B0 stub: li t2; jr t2; li t1; pad
            0x240A00B0, 0x01400008, 0x24090033, NOP,  # second stub
            0x27BDFFE8, 0xAFBF0010, jal(BASE + 0x10), NOP,  # caller
            0x8FBF0010, 0x27BD0018, JR_RA, NOP,
        )
        self.assertEqual(fallthrough_boundary(code, BASE + 0x10), BASE + 0x0C)
        extents = [FunctionExtent(BASE, BASE + 0x20), FunctionExtent(BASE + 0x20, BASE + 0x40)]
        rows = audit_image("T.EXE", code, extents, {})
        split = [row for row in rows if row["confidence"] == "call-split"]
        self.assertEqual(len(split), 1)
        self.assertEqual(split[0]["va"], f"0x{BASE + 0x10:08x}")
        self.assertEqual(split[0]["boundary"], f"split-owner-end=0x{BASE + 0x0C:08x}")

    def test_overlay_dispatch_pointer_carves_the_default_handler(self) -> None:
        # FDAT module shape: the lowest handler slot names a `jr ra; nop`
        # default handler right after the header, with no return before it.
        code = words(JR_RA, NOP, 0x27BDFFE8, 0xAFBF0010, 0x8FBF0010, 0x27BD0018, JR_RA, NOP)
        code.handlers = [BASE, BASE, BASE + 8]
        rows = audit_image("T.EXE", code, [FunctionExtent(BASE + 8, BASE + 0x20)], {})
        self.assertEqual([(row["va"], row["confidence"], row["return_va"], row["note"])
                          for row in rows],
                         [(f"0x{BASE:08x}", "call-target", f"0x{BASE:08x}",
                           "handler-pointer x2+first-unconditional-transfer")])

    def switch(self, second_case: int) -> ImageWords:
        """A table at BASE, a two-case switch at BASE + 8, a leaf after it."""
        f = BASE + 8
        return words(
            f + 0x2C, second_case,  # the jump table
            0x2C820002,  # sltiu v0,a0,2
            0x1040000D,  # beqz v0,default
            NOP,
            0x3C028001,  # lui v0,0x8001
            0x24420000,  # addiu v0,v0,0
            0x00041880,  # sll v1,a0,2
            0x00621821,  # addu v1,v1,v0
            0x8C620000,  # lw v0,0(v1)
            NOP,
            0x00400008, NOP,  # jr v0: a call carve stops here
            jump(f + 0x3C), 0x24020001,  # case 0: j default; li v0,1
            jump(f + 0x3C), 0x24020002,  # case 1: j default; li v0,2
            JR_RA, NOP,  # default
            JR_RA, NOP,  # the next admitted function
        )

    def test_carve_cut_at_its_switch_dispatch_owns_the_cases(self) -> None:
        f = BASE + 8
        code = self.switch(f + 0x34)
        extents = [FunctionExtent(f, f + 0x2C), FunctionExtent(f + 0x44, f + 0x4C)]
        self.assertEqual(switch_extension(code, extents[0], f + 0x44), f + 0x44)
        rows = [row for row in audit_image("T.EXE", code, extents, {})
                if row["confidence"] == "switch-extend"]
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["va"], f"0x{f + 0x2C:08x}")
        self.assertEqual(rows[0]["owner_va"], f"0x{f:08x}")
        self.assertEqual(rows[0]["return_va"], f"0x{f + 0x3C:08x}")

    def test_switch_case_outside_the_gap_does_not_extend(self) -> None:
        f = BASE + 8
        code = self.switch(f + 0x44)  # case 1 is the next function
        self.assertIsNone(switch_extension(code, FunctionExtent(f, f + 0x2C), f + 0x44))
        ends_at_return = words(JR_RA, NOP, JR_RA, NOP)
        self.assertIsNone(switch_extension(ends_at_return, FunctionExtent(BASE, BASE + 8),
                                           BASE + 0x10))

    def test_switch_extend_grows_the_owner_and_cuts_the_gap(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            config = Path(tmp)
            write_tsv(config / "functions.tsv", FUNCTION_FIELDS, [{
                "image": "GAME.EXE", "va": "0x80020000", "file_offset": "0xf800",
                "size": "0x2c", "body_size": "0x2c", "fragments": 1, "kind": "function",
                "confidence": "mips-call-carve", "name": "", "labels": "",
                "provenance": "scripts/kf/audit_functions.py", "note": "carve",
            }], [])
            write_tsv(config / "data.tsv", DATA_FIELDS, [{
                "image": "GAME.EXE", "va": "0x8002002c", "file_offset": "0xf82c",
                "size": "0x18", "kind": "unclassified", "confidence": "coverage-gap",
                "name": "", "datatype": "", "provenance": "payload-complement", "note": "",
            }], [])
            candidate = {
                "image": "GAME.EXE", "va": "0x8002002c", "owner_va": "0x80020000",
                "return_va": "0x8002003c", "confidence": "switch-extend", "note": "cases",
            }
            functions = proposed_functions(config, [candidate])
            self.assertEqual(len(functions), 1)
            self.assertEqual((functions[0]["size"], functions[0]["body_size"]),
                             ("0x44", "0x44"))
            self.assertEqual(proposed_data(config, [candidate]), [])

    def test_call_target_after_fallthrough_code_is_an_interior_entry(self) -> None:
        code = words(0x20A70060, 0x24060000, 0x8CE80048, NOP, JR_RA, NOP)
        self.assertIsNone(fallthrough_boundary(code, BASE + 8))

    def test_emitted_call_targets_keep_an_unrelated_candidate(self) -> None:
        # An admitted caller jals two BIOS stubs in a gap and a strong frame
        # above them; carving the stubs must not touch the frame's row.
        stubs = BASE + 0x34  # after the 12-word caller and a data word
        frame = stubs + 0x20
        caller = [0x27BDFFE8, 0xAFBF0010, jal(stubs), NOP, jal(stubs + 0x10), NOP,
                  jal(frame), NOP, 0x8FBF0010, 0x27BD0018, JR_RA, NOP]
        code = words(
            *caller, 0x12345678,  # a data word: no boundary before the stubs
            0x240A00B0, 0x01400008, 0x24090032, NOP,
            0x240A00B0, 0x01400008, 0x24090033, NOP,
            0x27BDFFE8, 0xAFBF0010, 0x8FBF0010, 0x27BD0018, JR_RA, NOP,
            *([0x24020001] * 8),
        )
        rows = audit_image("T.EXE", code, [FunctionExtent(BASE, BASE + 4 * len(caller))], {})
        starts = {row["va"]: row["confidence"] for row in rows}
        self.assertEqual(starts[f"0x{stubs:08x}"], "call-target")
        self.assertEqual(starts[f"0x{stubs + 0x10:08x}"], "call-target")
        self.assertEqual(starts[f"0x{frame:08x}"], "strong")

    def test_call_carve_runs_past_a_dispatch_its_branches_skip(self) -> None:
        # GAME.EXE 0x8003260c shape: the default branch targets code after
        # the table dispatch, so `jr v0` does not end the function.
        code = words(
            0x2C820002,  # sltiu v0,a0,2
            0x10400005,  # beqz v0,+5 (default, after the dispatch)
            NOP,
            0x8C220000,  # lw v0,0(at)
            NOP,
            0x00400008, NOP,  # jr v0
            0x24020001,  # case: li v0,1 (default lands here)
            JR_RA, NOP,
            0x24020002, JR_RA, NOP,  # the next function
        )
        self.assertEqual(transfer_end(code, BASE, BASE + 0x28), BASE + 0x28)
        self.assertEqual(transfer_end(code, BASE + 0x1C, BASE + 0x28), BASE + 0x28)
        # A tail call (`j` to or past the next start) still ends the carve.
        tail = words(0x24020001, jump(BASE + 0x10), NOP, NOP, JR_RA, NOP)
        self.assertEqual(transfer_end(tail, BASE, BASE + 0x10), BASE + 0x0C)
        # No length cap: a long straight run still reaches its return.
        long_run = words(*([0x24020001] * 0x500), JR_RA, NOP)
        self.assertEqual(transfer_end(long_run, BASE, BASE + 0x1408), BASE + 0x1408)

    def test_overlay_header_end_bounds_a_leaf_carve(self) -> None:
        # FDAT_029 0x801e8390 shape: a function right after the handler header
        # that no handler names and no return precedes.
        code = words(
            BASE + 0x1C, BASE + 0x1C,  # header: two handler slots
            0x24020001, 0x24030002, 0x00431021, JR_RA, NOP,  # unnamed function
            JR_RA, NOP,  # default handler
        )
        code.handlers = [BASE + 0x1C, BASE + 0x1C]
        code.header_end = BASE + 8
        rows = audit_image("T.EXE", code, [FunctionExtent(BASE + 0x1C, BASE + 0x24)], {})
        leaf = [row for row in rows if row["va"] == f"0x{BASE + 8:08x}"]
        self.assertEqual([(row["confidence"], row["boundary"], row["return_va"]) for row in leaf],
                         [("strong-leaf", "overlay-header-end", f"0x{BASE + 0x14:08x}")])

    def test_one_hoist_widening_per_signed_division(self) -> None:
        # A signed division has a `break 7` and a `break 6`; only the first
        # widens the hoist window (8 words + 10 per expansion = 18).
        division = (
            0x00C2001A, 0x14400002, NOP, 0x0007000D, 0x2401FFFF, 0x14410004,
            0x3C018000, 0x14C10002, NOP, 0x0006000D, 0x00003012,
        )
        frame = (0x27BDFFE8, 0xAFBF0010)
        within = words(JR_RA, NOP, *([0x34020001] * 7), *division, *frame)
        self.assertEqual(hoisted_start(within, BASE + 4 * 20), BASE + 8)
        beyond = words(JR_RA, NOP, *([0x34020001] * 9), *division, *frame)
        self.assertIsNone(hoisted_start(beyond, BASE + 4 * 22))

    def test_call_extend_window_widens_per_division(self) -> None:
        # The target lies 0x44 bytes above the admitted frame: past the plain
        # 0x40 limit, inside it once the division expansion is allowed for.
        division = (
            0x00C2001A, 0x14400002, NOP, 0x0007000D, 0x2401FFFF, 0x14410004,
            0x3C018000, 0x14C10002, NOP, 0x0006000D, 0x00003012,
        )
        code = words(
            0x27BDFFE8, 0xAFBF0010, jal(BASE + 0x20), NOP,  # caller
            0x8FBF0010, 0x27BD0018, JR_RA, NOP,
            0x00042400, *([0x34020001] * 5), *division,  # callee, 17 words
            0x27BDFFE8, 0xAFBF0010,  # admitted frame
            0x8FBF0010, 0x27BD0018, JR_RA, NOP,
        )
        frame = BASE + 0x20 + 0x44
        extents = [FunctionExtent(BASE, BASE + 0x20), FunctionExtent(frame, frame + 0x18)]
        rows = audit_image("T.EXE", code, extents, {})
        extend = [row for row in rows if row["confidence"] == "call-extend"]
        self.assertEqual([(row["va"], row["owner_va"], row["note"]) for row in extend],
                         [(f"0x{BASE + 0x20:08x}", f"0x{frame:08x}",
                           "jal-target x1+hoisted-above-admitted-start+division-checks x1")])


class ProposalTests(unittest.TestCase):
    def config(self, tmp: str, functions: list[tuple[int, int]], gaps: list[tuple[int, int]]) -> Path:
        config = Path(tmp)
        write_tsv(config / "functions.tsv", FUNCTION_FIELDS, [{
            "image": "GAME.EXE", "va": f"0x{va:08x}", "file_offset": f"0x{va - 0x80011000 + 0x800:x}",
            "size": f"0x{size:x}", "body_size": f"0x{size:x}", "fragments": 1,
            "kind": "function", "confidence": "mips-frame-carve", "name": "", "labels": "",
            "provenance": "scripts/kf/audit_functions.py", "note": "carve",
        } for va, size in functions], [])
        write_tsv(config / "data.tsv", DATA_FIELDS, [{
            "image": "GAME.EXE", "va": f"0x{va:08x}", "file_offset": f"0x{va - 0x80011000 + 0x800:x}",
            "size": f"0x{size:x}", "kind": "unclassified", "confidence": "coverage-gap",
            "name": "", "datatype": "", "provenance": "payload-complement", "note": "",
        } for va, size in gaps], [])
        return config

    def test_call_split_shortens_the_owner_and_leaves_its_padding_as_data(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            config = self.config(tmp, [(0x80020000, 0x40)], [])
            candidate = {
                "image": "GAME.EXE", "va": "0x80020020", "owner_va": "0x80020000",
                "return_va": "0x80020038", "boundary": "split-owner-end=0x8002001c",
                "confidence": "call-split", "note": "jal-target x1+owner-cannot-fall-through",
            }
            functions = proposed_functions(config, [candidate])
            self.assertEqual([(row["va"], row["size"]) for row in functions],
                             [("0x80020000", "0x1c"), ("0x80020020", "0x20")])
            self.assertEqual([(row["va"], row["size"]) for row in proposed_data(config, [candidate])],
                             [("0x8002001c", "0x4")])

    def test_call_extend_moves_the_start_within_the_division_widened_limit(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            config = self.config(tmp, [(0x80020044, 0x18)], [(0x80020000, 0x44)])
            candidate = {
                "image": "GAME.EXE", "va": "0x80020000", "owner_va": "0x80020044",
                "return_va": "0x80020054", "boundary": "previous-return",
                "confidence": "call-extend",
                "note": "jal-target x1+hoisted-above-admitted-start+division-checks x1",
            }
            functions = proposed_functions(config, [candidate])
            self.assertEqual([(row["va"], row["size"], row["file_offset"]) for row in functions],
                             [("0x80020000", "0x5c", "0xf800")])
            self.assertEqual(proposed_data(config, [candidate]), [])
            without = dict(candidate, note="jal-target x1+hoisted-above-admitted-start")
            with self.assertRaises(ValueError):
                proposed_functions(config, [without])

    def test_handler_pointer_carve_is_labelled_a_handler(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            config = self.config(tmp, [], [(0x80020000, 0x8)])
            candidate = {
                "image": "GAME.EXE", "va": "0x80020000", "owner_va": "",
                "return_va": "0x80020000", "boundary": "",
                "confidence": "call-target",
                "note": "handler-pointer x2+first-unconditional-transfer",
            }
            functions = proposed_functions(config, [candidate])
            self.assertEqual([row["confidence"] for row in functions], ["mips-handler-carve"])
            self.assertEqual(proposed_data(config, [candidate]), [])


class OverlayHeaderTests(unittest.TestCase):
    def test_handler_table_and_header_end_come_from_the_archive(self) -> None:
        header = struct.pack("<I32I", 5, *range(0x801E8390, 0x801E8390 + 32 * 4, 4))
        payload = header + struct.pack("<I", 0x25) + bytes(16)
        self.assertEqual(overlay_handlers("FDAT_002.BIN", payload),
                         list(range(0x801E8390, 0x801E8390 + 32 * 4, 4)))
        self.assertEqual(overlay_handlers("GAME.EXE", payload), [])
        self.assertEqual(overlay_header_end("FDAT_002.BIN"), 0x801E8390)
        self.assertIsNone(overlay_header_end("GAME.EXE"))
        with self.assertRaises(ValueError):
            overlay_handlers("FDAT_002.BIN", header[:0x40])


if __name__ == "__main__":
    unittest.main()
