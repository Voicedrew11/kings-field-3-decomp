"""Controls for reaching definitions and constant value sets (sema/values.py)."""

from __future__ import annotations

import struct
import unittest

from scripts.kf.sema.cfg import build_graph
from scripts.kf.sema.index import Binding
from scripts.kf.sema.values import ENTRY, ValueFlow


def words(*values):
    return struct.pack(f"<{len(values)}I", *values)


def flow(*code):
    payload = words(*code)
    function = Binding("GAME.EXE", 0x1000, len(payload), "text", "function", "fn", "",
                       "test", "test", body_size=len(payload))
    return ValueFlow(build_graph(function, payload, None))


NOP, JR_RA, JAL = 0, 0x03E00008, 0x0C000800  # jal 0x2000
S0, T0, V0 = 16, 8, 2


class ValueFlowTest(unittest.TestCase):
    def test_callee_saved_constant_survives_a_call(self):
        # lui s0,0; addiu s0,s0,0x100; jal; nop; jalr s0; nop; jr ra; nop
        values = flow(0x3C100000, 0x26100100, JAL, NOP, 0x0200F809, NOP, JR_RA, NOP)
        self.assertEqual(values.values(0x1010, S0), frozenset({0x100}))

    def test_caller_saved_register_is_unknown_after_a_call(self):
        values = flow(0x3C080000, 0x25080100, JAL, NOP, 0x0100F809, NOP, JR_RA, NOP)
        self.assertIsNone(values.values(0x1010, T0))
        (definition,) = values.definitions(0x1010, T0)
        self.assertEqual((definition.va, definition.call, definition.known),
                         (0x1008, True, False))

    def test_each_path_contributes_its_definition_and_constant(self):
        # lui s0,0; beq a0,zero,+2; addiu s0,s0,0x100; addiu s0,s0,0x10;
        # jalr s0; nop; jr ra; nop
        values = flow(0x3C100000, 0x10800002, 0x26100100, 0x26100010, 0x0200F809, NOP,
                      JR_RA, NOP)
        self.assertEqual(values.values(0x1010, S0), frozenset({0x100, 0x110}))
        self.assertEqual({d.va for d in values.definitions(0x1010, S0)}, {0x1008, 0x100C})

    def test_a_loaded_register_is_defined_by_its_load(self):
        # lui v0,0; lw v0,0x100(v0); nop; lw v0,0x40(v0); nop; jalr v0; nop; jr ra; nop
        values = flow(0x3C020000, 0x8C420100, NOP, 0x8C420040, NOP, 0x0040F809, NOP,
                      JR_RA, NOP)
        (definition,) = values.definitions(0x1014, V0)
        self.assertEqual(definition.va, 0x100C)
        self.assertTrue(definition.known)
        self.assertIsNone(values.values(0x1014, V0))  # a memory load is not a constant

    def test_an_incoming_register_reaches_from_entry(self):
        values = flow(0x0080F809, NOP, JR_RA, NOP)  # jalr a0
        (definition,) = values.definitions(0x1000, 4)
        self.assertEqual(definition.va, ENTRY)
        self.assertFalse(definition.known)

    def stack(self, delay=NOP, store_block_call=False):
        # addiu sp,-24; lui v0,0; addiu v0,v0,0x100; sw v0,20(sp);
        # beq a0,zero,+1 [DELAY]; jal; nop; lw t0,20(sp); nop; jalr t0; nop;
        # addiu sp,24; jr ra; nop
        branch = (JAL, NOP) if store_block_call else (0x10800001, delay)
        rest = (NOP, NOP) if store_block_call else (JAL, NOP)
        return flow(0x27BDFFE8, 0x3C020000, 0x24420100, 0xAFA20014, *branch, *rest,
                    0x8FA80014, NOP, 0x0100F809, NOP, 0x27BD0018, JR_RA, NOP)

    def test_a_local_stack_slot_keeps_its_constant_across_a_call(self):
        self.assertEqual(self.stack().values(0x1028, T0), frozenset({0x100}))

    def test_an_address_taken_slot_is_not_tracked(self):
        self.assertIsNone(self.stack(delay=0x27A40014).values(0x1028, T0))  # addiu a0,sp,20

    def test_a_slot_stored_before_a_call_in_its_block_is_an_outgoing_argument(self):
        self.assertIsNone(self.stack(store_block_call=True).values(0x1028, T0))

    def test_an_incomplete_graph_is_refused(self):
        payload = words(0x00400008, NOP)  # jr v0: an unresolved jump
        function = Binding("GAME.EXE", 0x1000, len(payload), "text", "function", "fn", "",
                           "test", "test", body_size=len(payload))
        with self.assertRaises(ValueError):
            ValueFlow(build_graph(function, payload, None))


if __name__ == "__main__":
    unittest.main()
