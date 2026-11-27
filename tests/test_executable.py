"""Direct source -> ASPSX -> PSYLINK -> CPE2X executable controls."""

from __future__ import annotations

import contextlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import struct
import tempfile
import unittest
from unittest import mock

from scripts.kf import executable
from scripts.kf.executable import build_image, compare, cpe_loads
from scripts.kf.manifest import Manifest, Profile, Unit
from scripts.kf.sema.image import RetailImage
from scripts.psxbuild.link import FIRST, IMAGE_LINKS
from scripts.psxbuild.link import build_image as link_image


class ComparisonTests(unittest.TestCase):
    def test_normal_build_does_not_load_retail_compare_or_refresh_readme(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            root = build / 'link/psx'
            root.mkdir(parents=True)
            report = {'image': 'PSX.EXE', 'linked': True, 'executable': str(root / 'PSX.EXE')}
            with (mock.patch.object(executable, 'BUILD', build),
                  mock.patch.object(executable, 'load_manifest'),
                  mock.patch.object(executable, 'build_image', return_value=report),
                  mock.patch.object(RetailImage, 'load', side_effect=AssertionError('retail used')),
                  mock.patch.object(executable, 'compare', side_effect=AssertionError('comparison used')),
                  mock.patch.object(executable, 'compare_executables', side_effect=AssertionError('diff used')),
                  mock.patch.object(executable, 'refresh_executable', side_effect=AssertionError('README used')),
                  contextlib.redirect_stdout(io.StringIO())):
                self.assertEqual(executable.main(['--image', 'psx']), 0)
            self.assertEqual(json.loads((root / 'build.json').read_text()), report)
            self.assertFalse((root / 'fuzzy-comparison.json').exists())

    def test_compare_only_skips_images_without_a_linked_exe_unless_selected(self):
        image = RetailImage.synthetic('PSX.EXE', 0x80010000, bytes(range(64)))
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            root = build / 'link/psx'
            root.mkdir(parents=True)
            (root / 'PSX.EXE').write_bytes(image.data)
            with (mock.patch.object(executable, 'BUILD', build),
                  mock.patch.object(RetailImage, 'load', return_value=image),
                  mock.patch.object(executable, 'refresh_executable'),
                  contextlib.redirect_stdout(io.StringIO()) as output):
                self.assertEqual(executable.main(['--compare-only']), 0)
            self.assertIn('GAME.EXE: no linked EXE yet; comparison skipped', output.getvalue())
            self.assertTrue((root / 'fuzzy-comparison.json').is_file())
            with (mock.patch.object(executable, 'BUILD', build),
                  mock.patch.object(RetailImage, 'load', return_value=image),
                  contextlib.redirect_stderr(io.StringIO()),
                  self.assertRaises(SystemExit)):
                executable.main(['--compare-only', '--image', 'game'])

    def test_file_tail_and_header_are_compared_without_masks(self):
        image = RetailImage.synthetic('PSX.EXE', 0x80010000, b'abcdefgh')
        self.assertTrue(compare(image.data, image)['file_equal'])
        for actual, count in ((image.data[:-3], 3), (image.data + b'\0\0', 2)):
            self.assertEqual(compare(actual, image)['differing_bytes'], count)
        changed = bytearray(image.data)
        changed[0x10] ^= 4
        changed[-1] ^= 1
        report = compare(bytes(changed), image)
        self.assertEqual(report['differing_bytes'], 2)
        self.assertEqual(report['header_differing_bytes'], 1)
        self.assertEqual(report['load_differing_bytes'], 1)
        self.assertIsNone(report['first_difference_va'])

    def test_payload_offset_is_qualified_by_selected_image_base(self):
        image = RetailImage.synthetic('OPEN.EXE', 0x80012000, b'abcdefgh')
        report = compare(image.data[:-1] + b'!', image)
        self.assertEqual(report['first_difference_va'], 0x80012007)

    def test_inferred_padding_mode_is_no_longer_available(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as error:
            executable.main(['--diagnostic-cpe-padding'])
        self.assertEqual(error.exception.code, 2)

    def test_cpe_inspection_keeps_load_order_and_rejects_truncation(self):
        entry = 0x80010020
        cpe = (b'CPE\x01\x08\x00\x03\x90\x00' + struct.pack('<I', entry)
               + b'\x01' + struct.pack('<II', entry, 3) + b'ABC'
               + b'\x01' + struct.pack('<II', 0x80010000, 4) + b'DATA' + b'\0')
        self.assertEqual(cpe_loads(cpe), (entry, [(entry, b'ABC'), (0x80010000, b'DATA')]))
        for broken in (cpe[:-1], cpe[:-2], cpe + b'\0', b'CPE\x01\xff\0'):
            with self.subTest(broken=broken), self.assertRaises(ValueError):
                cpe_loads(broken)

    def test_compare_only_preserves_executable_and_native_build_provenance(self):
        image = RetailImage.synthetic('GAME.EXE', 0x80012000, bytes(range(128)))
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            root = build / 'link/game'
            root.mkdir(parents=True)
            exe = root / 'GAME.EXE'
            exe.write_bytes(image.data)
            original_report = root / 'build.json'
            original_report.write_bytes(b'original native build provenance\n')
            with (mock.patch.object(executable, 'BUILD', build),
                  mock.patch.object(RetailImage, 'load', return_value=image),
                  mock.patch.object(executable, 'load_manifest', side_effect=AssertionError('manifest used')),
                  mock.patch.object(executable, 'build_image', side_effect=AssertionError('build used')),
                  mock.patch.object(executable, 'refresh_executable') as refresh,
                  contextlib.redirect_stdout(io.StringIO())):
                self.assertEqual(executable.main(['--image', 'game', '--compare-only']), 0)
                refresh.assert_called_once_with(build / 'link')
            self.assertEqual(exe.read_bytes(), image.data)
            self.assertEqual(original_report.read_bytes(), b'original native build provenance\n')
            report = json.loads((root / 'fuzzy-comparison.json').read_text())
            self.assertEqual(report['mode'], 'compare-existing')
            self.assertNotIn('linked', report)
            self.assertTrue(report['comparison']['file_equal'])
            self.assertEqual(report['layout_tolerant']['byte_similarity_percent'], 100)
            self.assertEqual(report['layout_tolerant']['retail_bytes'], 128)
            html = (root / 'fuzzy-comparison.html').read_text()
            self.assertIn('Movement map', html)
            self.assertIn('80012000', html)
            self.assertIn('id="minimum"', html)


class PartialImageTests(unittest.TestCase):
    @staticmethod
    def catalog(*addresses, vendored=()):
        from types import SimpleNamespace
        from scripts.kf.delink import Function
        functions = [Function('GAME.EXE', va, 8, 8, 1, f'func_{va:08x}', 'test', 'test',
                              provider='Sony' if va in vendored else '')
                     for va in addresses]
        return SimpleNamespace(functions={'GAME.EXE': functions})

    @staticmethod
    def manifest(*claimed):
        from scripts.kf.delink import Function
        profile = Profile('control', 'c', 'gcc260-sony', 'O2', 8, '2.21', ())
        functions = tuple(Function('GAME.EXE', va, 8, 8, 1, f'func_{va:08x}', 'test', 'test')
                          for va in claimed)
        return Manifest({profile.name: profile},
                        (Unit('game.control', 'GAME.EXE', 'src/game/control.c', profile.name, functions),))

    def test_unclaimed_game_function_skips_the_native_link(self):
        catalog = self.catalog(0x80014000, 0x80014008, 0x80014010, vendored=(0x80014010,))
        with (tempfile.TemporaryDirectory() as directory,
              mock.patch.object(executable, 'load_catalog', return_value=catalog),
              mock.patch.object(executable, 'link_image',
                                side_effect=AssertionError('partial image linked'))):
            report = build_image('GAME.EXE', self.manifest(0x80014000), Path(directory))
            self.assertEqual((report['linked'], report['phase']), (False, 'partial'))
            self.assertEqual(report['unclaimed_game_functions'], 1)
            self.assertEqual(json.loads((Path(directory) / 'build.json').read_text()), report)

    def test_every_game_function_claimed_links_natively(self):
        linked = {'image': 'GAME.EXE', 'linked': True, 'executable': 'GAME.EXE'}
        catalog = self.catalog(0x80014000, 0x80014010, vendored=(0x80014010,))
        with (tempfile.TemporaryDirectory() as directory,
              mock.patch.object(executable, 'load_catalog', return_value=catalog),
              mock.patch.object(executable, 'census_references', return_value=[]),
              mock.patch.object(executable, 'link_image', return_value=dict(linked)) as link):
            report = build_image('GAME.EXE', self.manifest(0x80014000), Path(directory))
            link.assert_called_once()
            self.assertTrue(report['linked'])

    @staticmethod
    def data_manifest(data=(), rodata=None):
        from scripts.kf.delink import Datum, Function
        profile = Profile('control', 'c', 'gcc260-sony', 'O2', 8, '2.21', ())
        functions = (Function('GAME.EXE', 0x80014000, 8, 8, 1, 'func_80014000', 'test', 'test'),)
        unit = Unit('game.control', 'GAME.EXE', 'src/game/control.c', profile.name, functions,
                    data=tuple(Datum(va, size, f'D_{va:08x}', 'load') for va, size in data),
                    rodata=rodata)
        return Manifest({profile.name: profile}, (unit,))

    def build_with_references(self, manifest, references):
        catalog = self.catalog(0x80014000, 0x80014010, vendored=(0x80014010,))
        linked = {'image': 'GAME.EXE', 'linked': True, 'executable': 'GAME.EXE'}
        with (tempfile.TemporaryDirectory() as directory,
              mock.patch.object(executable, 'load_catalog', return_value=catalog),
              mock.patch.object(executable, 'census_references', return_value=references),
              mock.patch.object(executable, 'link_image', return_value=dict(linked)) as link):
            report = build_image('GAME.EXE', manifest, Path(directory))
            if not report['linked']:
                self.assertEqual(json.loads((Path(directory) / 'build.json').read_text()), report)
            return report, link.call_count

    def test_unowned_data_reference_keeps_a_fully_claimed_image_partial(self):
        report, links = self.build_with_references(
            self.data_manifest(data=[(0x80080000, 4)]),
            [(0x80014004, 0x80080000), (0x80014004, 0x80090000), (0x80014004, 0x80090002)])
        self.assertEqual(links, 0)
        self.assertEqual((report['linked'], report['phase']), (False, 'partial'))
        self.assertEqual(report['unowned_data_references'], 2)
        self.assertNotIn('unclaimed_game_functions', report)

    def test_owned_library_foreign_or_unclaimed_site_references_do_not_block_the_link(self):
        report, links = self.build_with_references(
            self.data_manifest(data=[(0x80080000, 8)], rodata=(0x80012000, 0x10)),
            [(0x80014004, 0x80080004),      # a DATA claim
             (0x80014000, 0x8001200c),      # the unit's RODATA claim
             (0x80014004, 0x80014010),      # a library function
             (0x80014004, 0x1f800000),      # outside the image (scratchpad)
             (0x80015000, 0x80090000)])     # a site outside claimed code
        self.assertEqual(links, 1)
        self.assertTrue(report['linked'])

    def test_data_partial_image_is_not_a_build_failure(self):
        partial = {'image': 'GAME.EXE', 'linked': False, 'phase': 'partial',
                   'units': ['game.control'], 'unowned_data_references': 5}
        with (tempfile.TemporaryDirectory() as directory,
              mock.patch.object(executable, 'BUILD', Path(directory)),
              mock.patch.object(executable, 'load_manifest', return_value=self.manifest()),
              mock.patch.object(executable, 'build_image', return_value=partial),
              contextlib.redirect_stdout(io.StringIO()) as output):
            (Path(directory) / 'link/game').mkdir(parents=True)
            self.assertEqual(executable.main(['--image', 'game']), 0)
        self.assertIn('partial reconstruction (5 referenced data addresses without an owning unit)',
                      output.getvalue())

    def test_shipped_census_keeps_the_linking_images_on_the_link_path(self):
        # PSX, OPEN, END and the FDAT modules link byte-for-byte today, so the
        # real census must find no unowned data reference in them. (GAME's
        # count shrinks as its data gets owners; it is not pinned here.)
        from scripts.kf.manifest import load
        manifest = load()
        for name in executable.IMAGE_LAYOUTS:
            if name == 'GAME.EXE':
                continue
            units = [u for u in manifest.units if u.image == name and u.scope != 'vendored']
            with self.subTest(image=name):
                self.assertEqual(executable.unowned_data_references(name, units), [])

    def test_partial_image_is_not_a_build_failure(self):
        partial = {'image': 'GAME.EXE', 'linked': False, 'phase': 'partial',
                   'units': ['game.control'], 'unclaimed_game_functions': 3}
        with (tempfile.TemporaryDirectory() as directory,
              mock.patch.object(executable, 'BUILD', Path(directory)),
              mock.patch.object(executable, 'load_manifest', return_value=self.manifest()),
              mock.patch.object(executable, 'build_image', return_value=partial),
              contextlib.redirect_stdout(io.StringIO()) as output):
            (Path(directory) / 'link/game').mkdir(parents=True)
            self.assertEqual(executable.main(['--image', 'game']), 0)
        self.assertIn('partial reconstruction (3 game functions unclaimed)', output.getvalue())


class OverlayHostSymbolTests(unittest.TestCase):
    @staticmethod
    def resolve(*symbols):
        from scripts.kf.lnk import Object

        native = Object({}, dict(enumerate(symbols)), [])
        with mock.patch('scripts.kf.lnk.read', return_value=native):
            return executable.host_symbols('FDAT_002.BIN', [Path(__file__)])

    def test_references_resolve_through_the_host_address_book(self):
        from scripts.kf.lnk import Symbol

        self.assertEqual(self.resolve(Symbol('notify_enqueue', None), Symbol('handler', 1, 0x10),
                                      Symbol('DAT_801ba984', None), Symbol('DAT_8009c21c', None)),
                         {'notify_enqueue': 0x80041EEC, 'DAT_801ba984': 0x801BA984,
                          'DAT_8009c21c': 0x8009C21C})

    def test_module_addresses_and_unknown_names_are_not_host_symbols(self):
        from scripts.kf.lnk import Symbol

        for name in ('area_handler_noop', 'no_such_symbol'):
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, name):
                self.resolve(Symbol(name, None))


class ArchiveEntryTests(unittest.TestCase):
    def test_archive_entry_frame_matches_the_retail_checksum_rule(self):
        from scripts.psxbuild.link import equ_lines, overlay_reserved_size, pack_archive_entry

        payload = struct.pack('<3I', 0x15, 0x801E8398, 0x03E00008)
        entry = pack_archive_entry(payload)
        self.assertEqual(len(entry), 0x800)
        self.assertEqual(entry[:12], payload)
        self.assertEqual(entry[12:-4], bytes(0x800 - 16))
        self.assertEqual(struct.unpack_from('<I', entry, 0x7FC)[0],
                         (0x15 + 0x801E8398 + 0x03E00008 + 0x12345678) & 0xFFFFFFFF)
        # A payload that fills the sector up to the checksum word still fits
        # (FDAT_053.BIN's 0xff0 linked bytes end one word short of it); one
        # more word starts the next sector.
        self.assertEqual(len(pack_archive_entry(bytes(0x7FC))), 0x800)
        self.assertEqual(len(pack_archive_entry(bytes(0x800))), 0x1000)
        self.assertEqual(len(pack_archive_entry(bytes(0xFF0))), 0x1000)
        with self.assertRaises(ValueError):
            pack_archive_entry(b'\0\0')
        # Uninitialized storage past the linked bytes counts toward the entry.
        self.assertEqual(len(pack_archive_entry(bytes(0x7F0), 0xC)), 0x800)
        self.assertEqual(len(pack_archive_entry(bytes(0x7F0), 0x10)), 0x1000)
        with self.assertRaises(ValueError):
            pack_archive_entry(bytes(8), 2)
        map_text = (
            '    Start     Stop   Length      Obj Group            Section name\n'
            ' 801E8308 801E838F 00000088 801E8308 text             .rdata\n'
            ' 801E8390 801E8667 000002D8 801E8390 text             .text\n'
            ' 801E8668 801E8668 00000000 801E8668 bss              .sbss\n'
            ' 801E8668 801E8A87 00000420 801E8668 bss              .bss\n')
        self.assertEqual(overlay_reserved_size(map_text, 0x801E8308, 0x360), 0x420)
        self.assertEqual(overlay_reserved_size(map_text, 0x801E8308, 0x800), 0)
        self.assertEqual(overlay_reserved_size('', 0x801E8308, 0x360), 0)
        self.assertEqual(equ_lines({'func_80041eec': 0x80041EEC}),
                         ['func_80041eec\tequ\t$80041eec'])
        with self.assertRaises(ValueError):
            equ_lines({'bad name': 1})


@unittest.skipUnless(all(shutil.which(tool) for tool in ('cpppsx-260', 'cc1psx-260', 'dosbox-x'))
                     and all(os.environ.get(key) for key in (
                         'PSYQ_CC1PSX', 'PSYQ_ASPSX', 'PSYQ_PSYLINK', 'PSYQ_CPE2X', 'PSYQ_LIB',
                         'PSYQ_INCLUDE', 'PSYQ_MALLOC_OBJ')),
                     'requires the pinned compiler, original SDK tools and libraries')
class NativeBuildControls(unittest.TestCase):
    MAP_SECTION = re.compile(
        r'(?m)^ ([0-9A-F]{8}) [0-9A-F]{8} ([0-9A-F]{8}) [0-9A-F]{8} \w+ +(\.[a-z]+)\s*$')

    def manifest(self, root: Path, sources: tuple[str, ...], image: str = 'PSX.EXE',
                 small_data: int = 8) -> Manifest:
        profile = Profile('native-control', 'c', 'gcc260-sony', 'O2', small_data, '2.21', ())
        units = []
        for index, source in enumerate(sources):
            path = root / f'source{index}.c'
            path.write_text(source)
            units.append(Unit(f'{image[:-4].lower()}.control{index}', image, str(path), profile.name, ()))
        return Manifest({profile.name: profile}, tuple(units))

    def sections(self, mapping: str) -> dict[str, tuple[int, int]]:
        return {name: (int(start, 16), int(size, 16))
                for start, size, name in self.MAP_SECTION.findall(mapping)}

    def test_comparison_view_uses_the_identical_native_linker_input(self):
        from scripts.kf.compile import compile_source
        from scripts.kf.paths import REPO
        from scripts.kf.retail import write_tsv

        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = self.manifest(root, (
                'static int values[3] = {7, 9, 11}; int main(void) { return values[1]; }\n',))
            report = build_image('PSX.EXE', manifest, root / 'exe')
            self.assertTrue(report['linked'], report.get('error'))
            write_tsv(root / 'delink/psx/objects.tsv', ('object', 'scope'),
                      ({'object': 'objects/view.o', 'scope': 'decomp'},), ())
            unit, profile = manifest.units[0], next(iter(manifest.profiles.values()))
            compile_source(unit.source_path, unit.image, root / 'view.o', root / 'delink',
                           profile.optimization, profile.small_data, profile.aspsx_version,
                           (REPO / 'include', REPO / 'vendor/include', Path(os.environ['PSYQ_INCLUDE'])),
                           profile.cc1_flags, profile.compiler)
            self.assertEqual((root / 'view.OBJ').read_bytes(), (root / 'exe/U0000.OBJ').read_bytes())
            self.assertTrue((root / 'view.o').read_bytes().startswith(b'\x7fELF'))

    def test_source_objects_and_original_archives_link_without_retail_inputs(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = self.manifest(root, (
                'extern int helper(void); int main(void) { return helper(); }\n',
                'int value = 7; int helper(void) { return value; }\n',
            ))
            output = root / 'output'
            with mock.patch.object(RetailImage, 'load', side_effect=AssertionError('retail input used')):
                report = build_image('PSX.EXE', manifest, output)
            self.assertTrue(report['linked'], report.get('error'))
            self.assertFalse(report['output_rewritten'])
            self.assertEqual(report['retail_payload_inputs'], [])
            self.assertEqual(report['startup']['file'], '2MBYTE.OBJ')
            self.assertIsNone(report['allocator'])
            self.assertEqual([row['file'] for row in report['libraries']], ['LIBAPI.LIB', 'LIBSN.LIB'])
            commands = (output / 'LINK.LNK').read_text().splitlines()
            self.assertEqual([line.split('"')[1] for line in commands if '\tinclude\t' in line],
                             ['U0000.OBJ', 'U0001.OBJ', '2MBYTE.OBJ'])
            self.assertTrue((output / 'U0000.OBJ').read_bytes().startswith(b'LNK\x02'))
            self.assertTrue((output / 'U0001.OBJ').read_bytes().startswith(b'LNK\x02'))
            # Sony's 2.6.0 CC1PSX calls __main from main; 2MBYTE.OBJ defines it.
            self.assertRegex((output / 'U0000.S').read_text(), r'(?m)^\s*jal\s+__main\b')
            entry, loads = cpe_loads((output / 'PSX.CPE').read_bytes())
            actual = (output / 'PSX.EXE').read_bytes()
            self.assertEqual(actual[:8], b'PS-X EXE')
            # CPE2X 1.5 writes every header field: zero reserved words, gp0 and
            # save area, the 801ffff0 stack base and the /CA region title.
            self.assertEqual(actual[8:16] + actual[0x14:0x18], bytes(12))
            self.assertEqual(struct.unpack_from('<I', actual, 0x30)[0], 0x801ffff0)
            self.assertEqual(actual[0x34:0x4c], bytes(0x18))
            title = b'Sony Computer Entertainment Inc. for North America area'
            self.assertEqual(actual[0x4c:0x800], title + bytes(0x800 - 0x4c - len(title)))
            self.assertEqual(struct.unpack_from('<I', actual, 0x10)[0], entry)
            base = struct.unpack_from('<I', actual, 0x18)[0]
            for address, payload in loads:
                offset = 2048 + address - base
                self.assertEqual(actual[offset:offset + len(payload)], payload)
            end = max(address + len(payload) for address, payload in loads)
            self.assertEqual(actual[2048 + end - base:], bytes(len(actual) - 2048 - end + base))
            for row in report['libraries']:
                self.assertEqual((output / row['file']).read_bytes(), Path(row['path']).read_bytes())

    def test_overlay_startup_allocator_and_bss_use_native_linker_inputs(self):
        for image in ('OPEN.EXE', 'GAME.EXE', 'END.EXE'):
            with self.subTest(image=image), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                source = ('unsigned char buffer[8192]; int initialized_zero = 0;\n'
                          'int main(void) { buffer[4] = 7; '
                          'return buffer[2] + initialized_zero; }\n')
                output = root / 'output'
                manifest = self.manifest(root, (source,), image, small_data=0)
                with mock.patch.object(RetailImage, 'load', side_effect=AssertionError('retail input used')):
                    report = build_image(image, manifest, output)
                self.assertTrue(report['linked'], report.get('error'))
                layout = IMAGE_LINKS[image]
                self.assertFalse(report['layout']['proven'])
                self.assertEqual(report['startup']['file'], 'NONE2.OBJ')
                self.assertEqual((output / 'NONE2.OBJ').read_bytes(),
                                 Path(report['startup']['path']).read_bytes())
                self.assertEqual(report['allocator'] is not None, layout.allocator)
                self.assertEqual(report['retail_payload_inputs'], [])
                commands = (output / 'LINK.LNK').read_text().splitlines()
                explicit = [line.split('"')[1] for line in commands if '\tinclude\t' in line]
                startup = ['NONE2.OBJ', *(['MALLOC.OBJ'] if layout.allocator else [])]
                expected = ([*startup, 'U0000.OBJ'] if layout.startup_before == FIRST
                            else ['U0000.OBJ', *startup])
                self.assertEqual(explicit, expected)
                self.assertEqual([line.split('"')[1] for line in commands if 'inclib' in line],
                                 [library + '.LIB' for library in layout.libraries])
                actual = (output / image).read_bytes()
                base = struct.unpack_from('<I', actual, 0x18)[0]
                self.assertEqual(base, executable.IMAGE_LAYOUTS[image].load_address)
                sections = self.sections((output / (image[:-4] + '.MAP')).read_text())
                entry, loads = cpe_loads((output / (image[:-4] + '.CPE')).read_bytes())
                self.assertEqual(sections.get('.rdata', (base, 0))[0], base)
                self.assertGreaterEqual(sections['.bss'][1], 8192)
                self.assertTrue(all(address + len(data) <= sections['.bss'][0]
                                    for address, data in loads))
                self.assertLess(len(actual), 8192 + 0x1000)

    def test_image_without_a_link_model_fails_readably(self):
        with tempfile.TemporaryDirectory() as directory:
            report = link_image('FDAT.EXE', Path(directory) / 'out', [object()],
                                lambda *_: {}, repo=Path(directory), load_address=0x80010000)
            self.assertFalse(report['linked'])
            self.assertIn('FDAT.EXE: no link model', report['error'])

    def test_unresolved_source_symbol_fails_without_fallback_or_stale_executable(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = self.manifest(root, (
                'extern int missing_function(void); int main(void) { return missing_function(); }\n',
            ))
            output = root / 'output'
            output.mkdir()
            (output / 'PSX.EXE').write_bytes(b'stale result')
            report = build_image('PSX.EXE', manifest, output)
            self.assertFalse(report['linked'])
            self.assertEqual(report['phase'], 'link')
            self.assertIn('missing_function', report['error'])
            self.assertFalse((output / 'PSX.EXE').exists())
            self.assertEqual(report['retail_payload_inputs'], [])


if __name__ == '__main__':
    unittest.main()
