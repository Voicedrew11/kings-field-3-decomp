"""Copy named files out of the ISO 9660 tree of a raw 2352-byte-sector track.

    extract-iso9660.py TRACK.bin OUTDIR PATH...

Each PATH is an ISO directory path (`PSYQ/ASPSX.EXE`) or a directory whose
files are copied non-recursively (`PSX/LIB/`). Files keep their ISO names.
"""

import os
import struct
import sys


def main(track: str, output: str, wanted: list[str]) -> None:
    raw = open(track, 'rb').read()
    if len(raw) % 2352:
        raise SystemExit(f'{track}: not a raw 2352-byte-sector image')
    payload = {1: 16, 2: 24}[raw[15]]

    def read(lba: int, size: int) -> bytes:
        count = (size + 2047) // 2048
        return b''.join(raw[(lba + i) * 2352 + payload:(lba + i) * 2352 + payload + 2048]
                        for i in range(count))[:size]

    def entries(lba: int, size: int):
        data, position = read(lba, size), 0
        while position < len(data):
            length = data[position]
            if not length:
                position = (position // 2048 + 1) * 2048
                continue
            record = data[position:position + length]
            position += length
            identifier = record[33:33 + record[32]]
            if identifier in (b'\0', b'\1'):
                continue
            name = identifier.decode('ascii').split(';')[0]
            yield name, struct.unpack_from('<I', record, 2)[0], \
                struct.unpack_from('<I', record, 10)[0], bool(record[25] & 2)

    descriptor = read(16, 2048)
    if descriptor[1:6] != b'CD001':
        raise SystemExit(f'{track}: no ISO 9660 primary volume descriptor')
    root = descriptor[156:190]
    root_extent = struct.unpack_from('<I', root, 2)[0], struct.unpack_from('<I', root, 10)[0]

    def lookup(path: str):
        lba, size, directory = *root_extent, True
        for part in [p for p in path.split('/') if p]:
            match = next((e for e in entries(lba, size) if e[0] == part), None)
            if match is None:
                raise SystemExit(f'{track}: {path} not found')
            _, lba, size, directory = match
        return lba, size, directory

    os.makedirs(output, exist_ok=True)
    for path in wanted:
        lba, size, directory = lookup(path)
        if directory:
            for name, extent, length, is_directory in entries(lba, size):
                if not is_directory:
                    open(os.path.join(output, name), 'wb').write(read(extent, length))
        else:
            open(os.path.join(output, path.rsplit('/', 1)[-1]), 'wb').write(read(lba, size))


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], sys.argv[3:])
