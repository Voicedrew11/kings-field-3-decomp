"""The shared compiler and native SDK assembler used by every source build."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

C_COMPILERS = {
    # Sony's GNU C 2.60 CC1PSX.EXE ("GNU C 2.6.0 [AL 1.1, MM 40]") runs under
    # DOSBox; the Decompals 2.6.0 rebuild preprocesses and answers the
    # source-size probe, both optimizer-independent.
    "gcc260-sony": ("cpppsx-260", "cc1psx-260"),
    "gcc257-native": ("cpppsx-257", "cc1psx-257"),
    "gcc260-native": ("cpppsx-260", "cc1psx-260"),
    # The kit's extensionless CC1PSX (GCC 2.4.1) runs under DOSBox. Its
    # preprocessing and the source-size probe use the native 2.5.7 pair: both
    # are optimizer-independent and see the same headers and MIPS ABI.
    "gcc241-kit": ("cpppsx-257", "cc1psx-257"),
}
DOS_C_COMPILERS = frozenset({"gcc260-sony", "gcc241-kit"})
# The Psy-Q CCPSX 1.17 driver's preprocessor definitions for C with -O.
SONY_DRIVER_DEFINES = ('mips', '__mips__', '__mips', 'psx', '__psx__', '__psx',
                       '__EXTENSIONS__', '_MIPSEL', '__CHAR_UNSIGNED__',
                       '_LANGUAGE_C', 'LANGUAGE_C', '__OPTIMIZE__')
# The Runtime Library 3.3 CD's assembler: it reproduces the retail div/rem
# expansion and `li` -> `ori` with no hazard nop where 2.30 and 2.34 add one.
ASPSX_VERSION = '2.21'


def aspsx_command(stem: str, small_data: int) -> str:
    return f'aspsx -g -G{small_data} -o {stem}.OBJ {stem}.S > {stem}.TXT'


# Runtime Library 3.3 INLINE.H writes each GTE operation as placeholder words
# (`.word 0x0000007f`: below 0x2000 with SPECIAL funct 0x3e or 0x3f, which no
# R3000 instruction uses) that Sony's DMPSX 2.03 rewrites in the assembled
# object, from INLINE.TBL, into the COP2 transfers and commands. Objects
# without such words are never given to DMPSX.
GTE_WORD = re.compile(rb'(?m)^\s*\.word\s+0x([0-9a-fA-F]+)\s*$')


def gte_placeholder(value: int) -> bool:
    return value < 0x2000 and value & 0x3e == 0x3e


def uses_gte_inline(assembly: bytes) -> bool:
    return any(gte_placeholder(int(match.group(1), 16)) for match in GTE_WORD.finditer(assembly))


def dmpsx_commands(stem: str) -> list[str]:
    # The copy keeps ASPSX's object for dmpsx_verified.
    return [f'copy {stem}.OBJ {stem}.ASP > NUL', f'dmpsx {stem}.OBJ -b > {stem}.DMP']


def stage_dmpsx(root: Path) -> None:
    tool = Path(os.environ['PSYQ_DMPSX'])
    shutil.copyfile(tool, root / 'DMPSX.EXE')
    shutil.copyfile(tool.with_name('INLINE.TBL'), root / 'INLINE.TBL')


def dmpsx_verified(root: Path, stem: str) -> None:
    """Require DMPSX to have changed only GTE placeholder words of the object."""
    log = (root / (stem + '.DMP')).read_text(errors='replace')
    before, after = (root / (stem + '.ASP')).read_bytes(), (root / (stem + '.OBJ')).read_bytes()
    if 'DMPSX version' not in log or len(before) != len(after) or before == after:
        raise RuntimeError(f'{stem}: DMPSX rewrote no GTE placeholder: {log.strip()}')
    covered = set()
    for index in range(len(before)):
        if before[index] == after[index] or index in covered:
            continue
        for start in range(max(index - 3, 0), index + 1):
            word = before[start:start + 4]
            if len(word) == 4 and gte_placeholder(int.from_bytes(word, 'little')):
                covered.update(range(start, start + 4))
                break
        else:
            raise RuntimeError(f'{stem}: DMPSX changed object byte {index:#x} outside a GTE word')


def run_command(arguments, *, environment=None):
    result = subprocess.run(arguments, capture_output=True, env=environment, check=False)
    if result.returncode:
        detail = (result.stderr or result.stdout).decode(errors="replace")
        raise RuntimeError(f"command failed ({result.returncode}): {detail}")
    return result.stdout


# Real-mode hardware interrupts push their frames onto the running program's
# stack, so stack bytes a DOS tool never initializes depend on where the timer
# IRQ landed. CPE2X copies such bytes into the reserved EXE header words
# (offsets 8..9 change with the emulated CPU rate). `cycles=max` retunes the
# rate from host load, which made those bytes vary between builds; a fixed
# rate ties the IRQ schedule to the executed instruction count instead.
DOS_CYCLES = 'fixed 100000'


def dos_run(root: Path, commands: list[str], phase: str, settings: tuple[str, ...] = ()) -> None:
    batch = phase.upper() + '.BAT'
    (root / batch).write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
    environment = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy',
                       XDG_CONFIG_HOME=str(root / 'dosbox-config'))
    configuration = [argument for setting in settings for argument in ('-set', setting)]
    result = subprocess.run(['dosbox-x', '-silent', '-fastlaunch', '-set', 'sdl output=surface',
                             '-set', f'cpu cycles={DOS_CYCLES}', *configuration,
                             '-c', f'mount c "{root}"', '-c', 'c:', '-c', batch, '-exit'],
                            cwd=root, env=environment, capture_output=True, timeout=600)
    (root / (phase + '-dosbox.log')).write_bytes(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(f'DOSBox failed during {phase}: {result.returncode}')


def tool_succeeded(root: Path, log: str, product: str, magic: bytes) -> None:
    transcript = (root / log).read_text(errors='replace')
    if not re.search(r'(?m)^0 error\(s\)', transcript):
        errors = [line.strip() for line in transcript.splitlines() if 'Error' in line]
        detail = errors[0] if errors else next(iter(transcript.strip().splitlines()), 'no diagnostic output')
        raise RuntimeError(f'{log}: {len(errors)} native tool errors; first: {detail}')
    path = root / product
    if not path.is_file() or not path.read_bytes().startswith(magic):
        raise RuntimeError(f'{log}: native tool produced no valid {product}')


def dos_text(path: Path) -> None:
    # The DOS reader needs CRLF. No directives, instructions or literals change.
    path.write_bytes(path.read_bytes().replace(b'\r\n', b'\n').replace(b'\n', b'\r\n'))


def compile_c(source: Path, root: Path, stem: str, *, compiler: str,
              optimization: str, small_data: int, include_dirs: tuple[Path, ...],
              cc1_flags: tuple[str, ...] = (), defines: tuple[str, ...] = (),
              cc1_override: Path | None = None,
              environment: dict[str, str] | None = None, run=run_command) -> dict:
    cpp, cc1 = C_COMPILERS[compiler]
    preprocessed, assembly = root / (stem + '.I'), root / (stem + '.S')
    includes = [argument for path in include_dirs for argument in ('-I', str(path.resolve()))]
    # The name becomes the assembly's `.file`; older ASPSX builds hang on long
    # ones, so a checkout-relative spelling keeps deep worktrees buildable.
    driver = SONY_DRIVER_DEFINES if compiler == 'gcc260-sony' else ()
    cpp_command = [cpp, '-lang-c', '-undef', '-nostdinc', *includes,
                   *(f'-D{value}' for value in (*driver, *defines)),
                   os.path.relpath(source.resolve())]
    preprocessed.write_bytes(run(cpp_command))
    cc1_command = [str(cc1_override) if cc1_override else cc1, '-quiet', '-g', f'-{optimization}',
                   f'-G{small_data}', *cc1_flags, str(preprocessed), '-o', str(assembly)]
    probe = {}
    if compiler in DOS_C_COMPILERS:
        if cc1_override is not None:
            raise ValueError(f'{compiler} has no cc1 override')
        if compiler == 'gcc260-sony':
            shutil.copyfile(os.environ['PSYQ_CC1PSX'], root / 'CC1.EXE')
        else:
            stage_kit_gcc241(root / 'CC1.EXE')
        assembly.unlink(missing_ok=True)
        dos_command = ['CC1', '-quiet', '-g', f'-{optimization}', f'-G{small_data}', *cc1_flags,
                       preprocessed.name, '-o', assembly.name]
        dos_run(root, [' '.join([*dos_command, '>', stem + '.CC1'])], 'cc1')
        if not assembly.is_file() or not assembly.stat().st_size:
            log = root / (stem + '.CC1')
            detail = log.read_text(errors='replace').strip() if log.is_file() else ''
            raise RuntimeError(f'{compiler} CC1PSX produced no assembly: {detail or "no output"}')
        probe = {'size_probe_command': cc1_command}
        cc1_command = dos_command
    else:
        run(cc1_command, environment=environment)
    dos_text(assembly)
    gte = uses_gte_inline(assembly.read_bytes())
    return {'object': stem + '.OBJ', 'assembly': stem + '.S', 'log': stem + '.TXT',
            'preprocessor_command': cpp_command, 'compiler_command': cc1_command, **probe,
            'assembler_command': aspsx_command(stem, small_data),
            **({'gte_commands': dmpsx_commands(stem)} if gte else {}),
            'assembly_sha256': hashlib.sha256(assembly.read_bytes()).hexdigest()}


def stage_kit_gcc241(target: Path) -> None:
    """Write the kit's GCC 2.4.1 cc1 as a stubbed DOS executable.

    The Psy-Q 3.0 kit ships CC1PSX 2.4.1 as a bare go32 COFF image beside
    the go32-stubbed 2.6.0 CC1PSX.EXE. Prefixing that stub to the 2.4.1 image
    makes it runnable under DOSBox; neither compiler image is modified.
    """
    compiler = Path(os.environ['PSYQ_SDK']) / 'COMPILER'
    stubbed = (compiler / 'CC1PSX.EXE').read_bytes()
    image = (compiler / 'CC1PSX').read_bytes()
    last_page_bytes, pages = struct.unpack_from('<HH', stubbed, 2)
    stub_size = pages * 512 - ((512 - last_page_bytes) if last_page_bytes else 0)
    i386_coff = b'\x4c\x01'
    if (stubbed[:2] != b'MZ' or stubbed[stub_size:stub_size + 2] != i386_coff
            or image[:2] != i386_coff):
        raise RuntimeError('unexpected kit CC1PSX layout; cannot stage GCC 2.4.1')
    target.write_bytes(stubbed[:stub_size] + image)


def compile_classic(source: Path, root: Path, stem: str, *, include_dirs: tuple[Path, ...],
                    defines: tuple[str, ...] = (), **options) -> dict:
    """Use GCC's original MIPS headers and the driver definitions they require."""
    if options.get('compiler') != 'gcc257-native':
        raise ValueError('classic requires GCC 2.5.7 and its original headers')
    headers = Path(os.environ['PSYQ_C_INCLUDE'])
    provenance = {name: hashlib.sha256((headers / name).read_bytes()).hexdigest()
                  for name in ('stdarg.h', 'va-mips.h')}
    # gcc.c supplies the version; config/mips/psx.h selects little-endian MIPS.
    driver_defines = ('__GNUC__=2', '__GNUC_MINOR__=5', '__mips__', '__MIPSEL__')
    result = compile_c(source, root, stem, include_dirs=(headers, *include_dirs),
                       defines=(*driver_defines, *defines), **options)
    return {**result, 'compiler_headers': provenance}


def assemble(root: Path, stem: str, small_data: int) -> bytes:
    return assemble_many(root, (stem,), small_data)[stem]


def assemble_many(root: Path, stems: tuple[str, ...], small_data: int) -> dict[str, bytes]:
    shutil.copyfile(os.environ['PSYQ_ASPSX'], root / 'ASPSX.EXE')
    commands, gte = [], []
    for stem in stems:
        dos_text(root / (stem + '.S'))
        (root / (stem + '.OBJ')).unlink(missing_ok=True)
        commands.append(aspsx_command(stem, small_data))
        if uses_gte_inline((root / (stem + '.S')).read_bytes()):
            commands.extend(dmpsx_commands(stem))
            gte.append(stem)
    if gte:
        stage_dmpsx(root)
    dos_run(root, commands, 'asm')
    for stem in stems:
        tool_succeeded(root, stem + '.TXT', stem + '.OBJ', b'LNK\x02')
        if stem in gte:
            dmpsx_verified(root, stem)
    return {stem: (root / (stem + '.OBJ')).read_bytes() for stem in stems}
