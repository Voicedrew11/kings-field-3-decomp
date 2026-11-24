from __future__ import annotations

import struct
import unittest

from scripts.kf.audit_functions import (
    FunctionExtent,
    ImageWords,
    audit_image,
    fallthrough_boundary,
    hoisted_start,
    matching_return,
    frame_at,
)
from scripts.kf.retail import ImageLayout


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

    def test_call_target_after_fallthrough_code_is_an_interior_entry(self) -> None:
        code = words(0x20A70060, 0x24060000, 0x8CE80048, NOP, JR_RA, NOP)
        self.assertIsNone(fallthrough_boundary(code, BASE + 8))


if __name__ == "__main__":
    unittest.main()
