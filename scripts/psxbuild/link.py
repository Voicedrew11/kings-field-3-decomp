"""Link PlayStation programs with the Psy-Q SDK."""

from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess

from .sdk import dmpsx_verified, dos_run, stage_dmpsx, tool_succeeded
from .sdk_compat import library_input


@dataclass(frozen=True)
class ImageLink:
    """How one King's Field III program is linked.

    `libraries` is the `inclib` order. PSYLINK visits the archives in this
    order and returns to the first one after every member it takes, so the
    groups of members from each archive follow one another in the linked text
    in this order, and a member's dependencies on an archive listed earlier
    follow it directly. Archives no member is taken from have no effect.
    `startup` names the startup object and `startup_before` the unit it is
    linked directly before (None: after every unit; FIRST: before every
    unit). `allocator` links MALLOC.OBJ directly after the startup object,
    or directly after the unit `allocator_after` names when that unit is
    built.
    `proven` is True only for a layout that has rebuilt its retail program
    byte-identically.
    """

    libraries: tuple[str, ...]
    startup: str
    startup_before: str | None = None
    allocator: bool = False
    allocator_after: str | None = None
    proven: bool = False
    evidence: str = ''


FIRST = ''
IMAGE_LINKS = {
    # main, then 2MBYTE.OBJ (__main at 0x80010118, the entry point after it),
    # then the LIBAPI stubs and LIBSN SNDEF's _stacksize/_ramsize.
    'PSX.EXE': ImageLink(('LIBAPI', 'LIBSN'), startup='2MBYTE.OBJ', proven=True,
                         evidence='retail PSX.EXE byte-identical with Runtime Library 3.3'),
    # UNPROVEN: the overlays' member groups were read from Ghidra signature
    # matches (the scoping layout census), not from a link, and these programs
    # link Library 3.4 (GAME, OPEN) or a post-3.3 LIBGPU (END), not the 3.3
    # archives. GAME has NONE2, MALLOC and a CARD object (_card_clear, not
    # modelled) before the game code, and links LIBPRESS; OPEN and END put
    # NONE2 after their first game units.
    'GAME.EXE': ImageLink(('LIBCARD', 'LIBCD', 'LIBSPU', 'LIBSND', 'LIBPRESS', 'LIBGTE',
                           'LIBETC', 'LIBAPI', 'LIBC', 'LIBGPU'),
                          startup='NONE2.OBJ', startup_before=FIRST, allocator=True,
                          evidence='signature census: NONE2 MALLOC CARD | game | CD SPU/SND PRESS '
                                   'GTE ETC API GPU'),
    'OPEN.EXE': ImageLink(('LIBAPI', 'LIBC', 'LIBPRESS', 'LIBGPU', 'LIBGTE', 'LIBCD', 'LIBETC',
                           'LIBSND', 'LIBSPU', 'LIBCARD'),
                          startup='NONE2.OBJ', startup_before='open.movie', allocator=True,
                          allocator_after='open.card',
                          evidence='signature census: game | NONE2 | game | MALLOC | PRESS GPU GTE '
                                   'CD ETC SND API SPU'),
    'END.EXE': ImageLink(('LIBAPI', 'LIBC', 'LIBPRESS', 'LIBGPU', 'LIBGTE', 'LIBCD', 'LIBETC',
                          'LIBSND', 'LIBSPU'),
                         startup='NONE2.OBJ', startup_before='end.movie', allocator=True,
                         allocator_after='end.movie',
                         evidence='signature census: game | NONE2 | game | MALLOC | API PRESS GPU '
                                  'GTE CD ETC SND SPU'),
}
# A headerless FDAT module is linked on its own at the address GAME.EXE loads
# it to: no startup object, allocator or library (the modules call GAME.EXE's
# code in place), its references into GAME.EXE defined as absolute symbols,
# PSYLINK's pure binary output (`/p`) instead of a CPE, framed as an FDAT.T
# entry.
OVERLAY_LINK = ImageLink((), startup='',
                         evidence='FDAT modules: own code only, jal into GAME.EXE text')
# FDAT.T frames each entry: the linked bytes, zeros through the module's
# uninitialized storage and up to the sector multiple that leaves room for one
# trailing word, and that word, 0x12345678 plus the 32-bit sum of every
# preceding word (all 28 retail code entries agree).
ARCHIVE_SECTOR = 0x800
ARCHIVE_CHECKSUM_SEED = 0x12345678
ENTRY = '__SN_ENTRY_POINT'
# Runtime Library 3.3 CD PSX/LIB objects.
STARTUP_SHA256 = {
    '2MBYTE.OBJ': 'b4ed8b2e76bc841f7a81132906d846877aefa8f6233fb738600c8b1ebd032a46',
    'NONE2.OBJ': '38662381b57cdbebb8d083842fd23914bad1d4c66303beb871f2f5248c38774c',
}
# The 3.3 CD's MALLOC.OBJ, byte-identical to the Release 2.5 floppies' object.
MALLOC_OBJECT_SHA256 = '628e405fd0e3acfff2ce9d4a15d481f0aa36398c14e9eae0a82b7ff0a86a74c9'
# PSYLINK 2.27 (Runtime Library 3.3 CD). PSYLINK 2.37 links PSX.EXE differently.
LINKER_SHA256 = '39c9e16ba84ddfb366004dc36f9c0b5a30f9908e56f8fca4b06ddb6eb748243b'
# CPE2X 1.5 writes every header field; /CA selects the "North America area"
# title that the retail SLUS-00255 headers carry.
CONVERTER_SHA256 = '777caca88327cd18d4f92b43439575c1759381690d49f14177805a94b8ecbef6'
CONVERTER_REGION = '/CA'
# .rdata before .text: PSX.EXE's string literals start its text group.
SECTIONS = ('text\tgroup', 'bss\tgroup\tbss',
            '\tsection\t.rdata,text', '\tsection\t.text,text',
            '\tsection\t.data,text', '\tsection\t.sdata,text',
            '\tsection\t.sbss,bss', '\tsection\t.bss,bss')


def image_link(name: str) -> ImageLink:
    try:
        return IMAGE_LINKS[name]
    except KeyError:
        raise ValueError(f'{name}: no link model; add it to IMAGE_LINKS in '
                         f'scripts/psxbuild/link.py (known: {", ".join(IMAGE_LINKS)})') from None


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pack_archive_entry(payload: bytes, reserved: int = 0) -> bytes:
    """Frame one linked module as its FDAT.T entry (see ARCHIVE_CHECKSUM_SEED).

    `reserved` is the extent of the module's uninitialized storage past the
    linked bytes: the entry covers it with zeros before the checksum word.
    """
    if len(payload) % 4 or reserved % 4 or reserved < 0:
        raise ValueError('an archive entry payload must be whole words')
    size = -(-(len(payload) + reserved + 4) // ARCHIVE_SECTOR) * ARCHIVE_SECTOR
    body = payload + bytes(size - 4 - len(payload))
    total = sum(struct.unpack(f'<{len(body) // 4}I', body)) + ARCHIVE_CHECKSUM_SEED
    return body + struct.pack('<I', total & 0xFFFFFFFF)


def overlay_reserved_size(map_text: str, load_address: int, linked_size: int) -> int:
    """Bytes of the `bss` group that lie past a module's linked bytes, from
    PSYLINK's map (`Start Stop Length Obj Group Section` rows), rounded up to
    whole words."""
    end = load_address + linked_size
    for line in map_text.splitlines():
        fields = line.split()
        if len(fields) == 6 and fields[4] == 'bss' and int(fields[2], 16):
            end = max(end, int(fields[1], 16) + 1)
    return -(-(end - load_address - linked_size) // 4) * 4


def equ_lines(symbols: dict[str, int]) -> list[str]:
    """PSYLINK command-file definitions of absolute host symbols."""
    for name in symbols:
        if not name or not (name[0].isalpha() or name[0] in '_$') or not all(
                c.isalnum() or c in '_$' for c in name):
            raise ValueError(f'invalid absolute symbol name {name!r}')
    return [f'{name}\tequ\t${address:08x}' for name, address in sorted(symbols.items())]


def _pinned_object(source: Path, expected: str, root: Path, name: str, provenance: str) -> dict:
    if file_hash(source) != expected:
        raise ValueError(f'{source}: expected the pinned {name} (sha256 {expected[:12]}...)')
    shutil.copyfile(source, root / name)
    return {'file': name, 'path': str(source), 'sha256': expected, 'provenance': provenance}


def build_image(name, root, units, compile_one, *, repo, load_address,
                host_symbols=None) -> dict:
    """Compile and link a program using its source units and SDK libraries.

    `host_symbols` selects the headerless module link (OVERLAY_LINK): it is
    called with the native objects and returns the absolute addresses of the
    host-program symbols they reference but do not define.
    """
    root = root.resolve()
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)
    report = {'image': name, 'linked': False, 'phase': 'compile', 'units': [],
              'libraries': [], 'startup': None, 'allocator': None, 'tools': {}}
    overlay = host_symbols is not None
    try:
        layout = OVERLAY_LINK if overlay else image_link(name)
        report['layout'] = {'proven': layout.proven, 'evidence': layout.evidence}
        tools = {'ASPSX.EXE': Path(os.environ['PSYQ_ASPSX']),
                 'PSYLINK.EXE': Path(os.environ['PSYQ_PSYLINK']),
                 'CPE2X.EXE': Path(os.environ['PSYQ_CPE2X'])}
        for tool, source in tools.items():
            shutil.copyfile(source, root / tool)
            report['tools'][tool] = {'path': str(source), 'sha256': file_hash(source)}
        if report['tools']['PSYLINK.EXE']['sha256'] != LINKER_SHA256:
            raise ValueError(f'{tools["PSYLINK.EXE"]}: expected the pinned PSYLINK 2.27')
        if report['tools']['CPE2X.EXE']['sha256'] != CONVERTER_SHA256:
            raise ValueError(f'{tools["CPE2X.EXE"]}: expected the pinned CPE2X 1.5')
        if not units:
            raise ValueError(f'{name}: no C source units')
        for index, unit in enumerate(units):
            report['units'].append(compile_one(unit, root, index))
        library_dir = Path(os.environ['PSYQ_LIB'])
        startup = []
        if layout.startup:
            report['startup'] = _pinned_object(
                library_dir / layout.startup, STARTUP_SHA256[layout.startup], root,
                layout.startup, 'Runtime Library 3.3 CD PSX/LIB object')
            startup.append(layout.startup)
        if layout.allocator:
            report['allocator'] = _pinned_object(
                Path(os.environ['PSYQ_MALLOC_OBJ']), MALLOC_OBJECT_SHA256, root, 'MALLOC.OBJ',
                'Runtime Library 3.3 CD PSX/LIB/MALLOC.OBJ (= Release 2.5 object)')
            startup.append('MALLOC.OBJ')
        report['phase'] = 'assemble'
        assembler_commands = [command for u in report['units'] if 'assembler_command' in u
                              for command in (u['assembler_command'], *u.get('gte_commands', ()))]
        if any('gte_commands' in u for u in report['units']):
            tool = Path(os.environ['PSYQ_DMPSX'])
            stage_dmpsx(root)
            report['tools']['DMPSX.EXE'] = {'path': str(tool), 'sha256': file_hash(tool)}
        if assembler_commands:
            dos_run(root, assembler_commands, 'asm')
        for unit in report['units']:
            if 'assembler_command' in unit:
                tool_succeeded(root, unit['log'], unit['object'], b'LNK\x02')
            if 'gte_commands' in unit:
                dmpsx_verified(root, unit['object'].removesuffix('.OBJ'))
            elif not (root / unit['object']).read_bytes().startswith(b'LNK\x02'):
                raise ValueError('compiler produced no native object')
            unit['object_sha256'] = file_hash(root / unit['object'])
        for library in layout.libraries:
            filename = library + '.LIB'
            source = library_dir / filename
            data, corrections = library_input(filename, source.read_bytes())
            (root / filename).write_bytes(data)
            report['libraries'].append({
                'file': filename, 'path': str(source), 'sha256': file_hash(source),
                'link_input_sha256': hashlib.sha256(data).hexdigest(),
                'corrections': corrections,
            })
        report['phase'] = 'link'
        stem = name.rsplit('.', 1)[0]
        objects = [u['object'] for u in report['units']]
        names = [u['unit'] for u in report['units']]
        position = (0 if layout.startup_before == FIRST else
                    names.index(layout.startup_before) if layout.startup_before in names
                    else len(objects))
        allocator_unit = (objects[names.index(layout.allocator_after)]
                          if layout.allocator and layout.allocator_after in names else None)
        if allocator_unit:
            startup.remove('MALLOC.OBJ')
        objects[position:position] = startup
        if allocator_unit:
            after = objects.index(allocator_unit) + 1
            objects[after:after] = ['MALLOC.OBJ']
        if report['startup'] is not None:
            report['startup']['after_source_units'] = position
        if overlay:
            absolute = host_symbols([root / u['object'] for u in report['units']])
            report['host_symbols'] = {key: f'{value:#010x}'
                                      for key, value in sorted(absolute.items())}
            tail = equ_lines(absolute)
        else:
            tail = [f'\tregs\tpc={ENTRY}']
        commands = [f'\torg\t${load_address:08x}', *SECTIONS,
                    *(f'\tinclude\t"{obj}"' for obj in objects),
                    *(f'\tinclib\t"{library["file"]}"' for library in report['libraries']),
                    *tail]
        (root / 'LINK.LNK').write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
        if overlay:
            report['linker_command'] = (
                f'psylink /c /m /p @LINK.LNK,{stem}.RAW,{stem}.SYM,{stem}.MAP > LINK.TXT')
            dos_run(root, [report['linker_command']], 'link')
            tool_succeeded(root, 'LINK.TXT', stem + '.RAW', b'')
            report['phase'] = 'pack'
            raw = (root / (stem + '.RAW')).read_bytes()
            map_path = root / (stem + '.MAP')
            reserved = overlay_reserved_size(
                map_path.read_text(errors='replace') if map_path.is_file() else '',
                load_address, len(raw))
            (root / name).write_bytes(pack_archive_entry(raw, reserved))
            report.update(linked=True, phase='complete', executable=str(root / name),
                          raw=str(root / (stem + '.RAW')), raw_size=len(raw),
                          reserved_size=reserved)
            (root / 'build.json').write_text(json.dumps(report, indent=2) + '\n')
            return report
        report['linker_command'] = (
            f'psylink /c /m @LINK.LNK,{stem}.CPE,{stem}.SYM,{stem}.MAP > LINK.TXT')
        dos_run(root, [report['linker_command']], 'link')
        tool_succeeded(root, 'LINK.TXT', stem + '.CPE', b'CPE\x01')
        report['phase'] = 'convert'
        report['converter_command'] = f'cpe2x {CONVERTER_REGION} {stem}.CPE > CONVERT.TXT'
        dos_run(root, [report['converter_command']], 'convert')
        executable = root / name
        if not executable.is_file():
            raise RuntimeError('CPE2X produced no executable; see CONVERT.TXT')
        actual = executable.read_bytes()
        if len(actual) < 2048 or actual[:8] != b'PS-X EXE':
            raise ValueError('CPE2X produced an invalid executable')
        report.update(linked=True, phase='complete', executable=str(executable),
                      cpe=str(root / (stem + '.CPE')))
    except (KeyError, OSError, RuntimeError, ValueError, subprocess.SubprocessError) as error:
        (root / name).unlink(missing_ok=True)
        report['error'] = str(error)
    (root / 'build.json').write_text(json.dumps(report, indent=2) + '\n')
    return report
