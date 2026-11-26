"""DMPSX 2.03 rewrites Runtime Library 3.3 INLINE.H GTE placeholders after ASPSX."""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import struct
import tempfile
import unittest

from scripts.kf.lnk import read
from scripts.psxbuild.sdk import (
    assemble_many, compile_c, dmpsx_verified, gte_placeholder, uses_gte_inline,
)

GTE_SOURCE = '''
#include <INLINE.H>
void project(long *vertex, long *screen, long *depth)
{
    gte_ldv0(vertex);
    gte_rtps();
    gte_stsxy(screen);
    gte_stszotz(depth);
}
int facing(long *a, long *b, long *c)
{
    int area;
    gte_ldv3(a, b, c);
    gte_rtpt();
    gte_nclip();
    gte_stopz(&area);
    return area;
}
'''
RTPS, RTPT, NCLIP = 0x4a180001, 0x4a280030, 0x4b400006


class PlaceholderTests(unittest.TestCase):
    def test_placeholders_are_low_special_words_with_funct_3e_or_3f(self):
        self.assertTrue(gte_placeholder(0x7f))
        self.assertTrue(gte_placeholder(0x13e))
        self.assertTrue(gte_placeholder(0x1eff))
        for value in (0x0, 0x7d, 0x2000 | 0x3f, 0x03e00008, RTPS):
            self.assertFalse(gte_placeholder(value), hex(value))
        self.assertTrue(uses_gte_inline(b'\tjal f\r\n\t.word\t0x0000007f\r\n'))
        self.assertFalse(uses_gte_inline(b'\t.word\t0x00000010\r\n\t.word\t0x80010000\r\n'))

    def test_verifier_accepts_only_rewritten_placeholder_words(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            before = b'LNK\x02\x06\x02\x00' + struct.pack('<3I', 0x24020001, 0x7f, 0x7e)
            rewritten = before[:-8] + struct.pack('<2I', 0xc9800000, 0xc9810004)
            (root / 'U.ASP').write_bytes(before)
            (root / 'U.DMP').write_text('DMPSX version 2.03\n')
            (root / 'U.OBJ').write_bytes(rewritten)
            dmpsx_verified(root, 'U')
            (root / 'U.OBJ').write_bytes(before)
            with self.assertRaisesRegex(RuntimeError, 'rewrote no GTE placeholder'):
                dmpsx_verified(root, 'U')
            (root / 'U.OBJ').write_bytes(rewritten[:7] + b'\x25' + rewritten[8:])
            with self.assertRaisesRegex(RuntimeError, 'outside a GTE word'):
                dmpsx_verified(root, 'U')


@unittest.skipUnless(shutil.which('cpppsx-260') and shutil.which('dosbox-x')
                     and all(os.environ.get(key) for key in (
                         'PSYQ_CC1PSX', 'PSYQ_ASPSX', 'PSYQ_DMPSX', 'PSYQ_INCLUDE')),
                     'requires the pinned compiler, ASPSX and DMPSX')
class NativeDmpsxTests(unittest.TestCase):
    def test_inline_gte_macros_become_cop2_code_in_the_object(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'gte.c'
            source.write_text(GTE_SOURCE)
            result = compile_c(source, root, 'GTE', compiler='gcc260-sony', optimization='O2',
                               small_data=8, include_dirs=(Path(os.environ['PSYQ_INCLUDE']),))
            self.assertIn('gte_commands', result)
            native = assemble_many(root, ('GTE',), 8)['GTE']
            text = next(s for s in read(native).sections.values() if s.name == '.text').data
            words = struct.unpack(f'<{len(text) // 4}I', bytes(text))
            self.assertFalse([hex(w) for w in words if gte_placeholder(w)])
            for command in (RTPS, RTPT, NCLIP):
                self.assertIn(command, words)
            # gte_ldv0: `move $12, vertex`, then lwc2 $0, 0($12); lwc2 $1, 4($12).
            self.assertIn(0xc9800000, words)
            self.assertIn(0xc9810004, words)
            self.assertNotEqual((root / 'GTE.ASP').read_bytes(), native)

    def test_objects_without_placeholders_skip_dmpsx(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'plain.c'
            source.write_text('int twice(int value) { return value * 2; }\n')
            result = compile_c(source, root, 'PLAIN', compiler='gcc260-sony', optimization='O2',
                               small_data=8, include_dirs=())
            self.assertNotIn('gte_commands', result)
            assemble_many(root, ('PLAIN',), 8)
            self.assertFalse((root / 'PLAIN.ASP').exists())
            self.assertFalse((root / 'DMPSX.EXE').exists())


if __name__ == '__main__':
    unittest.main()
