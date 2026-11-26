{ pkgs, sdkBuilder }:
let
      # Psy-Q PS-X Development System Release 3.0 kit; files dated 1995-04-08
      # despite the Archive.org item title. Its libraries carry the exact Sony
      # RCS revisions embedded in the retail JP/US executables.
      psyqKit = pkgs.fetchurl {
        name = "psyq-3.0-psx.zip";
        url = "https://archive.org/download/psyq_psx_toolchain_april_08_1994/psx.zip";
        hash = "sha256-QWJBY3zbAnO2vZNrCrVifIunIl7iuYBmzztQ+wnleJk=";
      };

      # Native Decompals rebuild used only as a practical code-generation probe.
      # It corresponds to GCC 2.6.0's PSX target, but is not evidence that this
      # rebuilt host binary (or either staged historical 2.6.0 binary) built KF.
      gcc260NativeArchive = pkgs.fetchurl {
        name = "decompals-old-gcc-0.17-gcc-2.6.0-psx.tar.gz";
        url = "https://github.com/decompals/old-gcc/releases/download/0.17/gcc-2.6.0-psx.tar.gz";
        hash = "sha256-NY2slJ8PVmr5xq+Tu6DUrATVbKnfgMWL1m2Bu7UN7Os=";
      };
      gcc260Native = pkgs.runCommand "decompals-gcc-2.6.0-psx-0.17" {
        nativeBuildInputs = [ pkgs.gnutar pkgs.gzip ];
      } ''
        mkdir -p "$out/bin"
        tar -xzf ${gcc260NativeArchive} -C "$out/bin"
        chmod +x "$out/bin"/*
      '';

      # Second native probe. GCC 2.5.7 is the closest available rebuild to the
      # 1994 Psy-Q compiler family; its text epilogue and load hoisting match
      # retail forms that 2.6.0 cannot emit, but it is still a probe.
      gcc257NativeArchive = pkgs.fetchurl {
        name = "decompals-old-gcc-0.17-gcc-2.5.7-psx.tar.gz";
        url = "https://github.com/decompals/old-gcc/releases/download/0.17/gcc-2.5.7-psx.tar.gz";
        hash = "sha256-DbH7QDx2blGwlDVLmhNqbB8BRXqkHMyiisRXt8tn2HE=";
      };
      gcc257Native = pkgs.runCommand "decompals-gcc-2.5.7-psx-0.17" {
        nativeBuildInputs = [ pkgs.gnutar pkgs.gzip ];
      } ''
        mkdir -p "$out/bin"
        tar -xzf ${gcc257NativeArchive} -C "$out/bin"
        chmod +x "$out/bin"/*
      '';

      gcc257SourceArchive = pkgs.fetchurl {
        url = "https://www.nic.funet.fi/index/gnu/funet/historical-funet-gnu-area-from-early-1990s/gcc-2.5.7.tar.gz";
        hash = "sha256-Y0W+QiNeXsTESNZYflUoPaPuV3YdtdINegpI1jmH/I4=";
      };
      gcc257Headers = pkgs.runCommand "gcc-2.5.7-mips-headers" {
        nativeBuildInputs = [ pkgs.gnutar pkgs.gzip ];
      } ''
        mkdir -p "$out/include"
        tar -xzf ${gcc257SourceArchive} --strip-components=1 -C "$out/include" \
          gcc-2.5.7/gstdarg.h gcc-2.5.7/va-mips.h
        mv "$out/include/gstdarg.h" "$out/include/stdarg.h"
      '';

      # The Psy-Q 3.0 kit tree (files dated 1995-04-08), inherited from King's
      # Field II. KF3 links none of it: it stays only as the analysis corpus of
      # the vendored-function seed and FID census and for the gcc241-kit probe.
      psyqSdk = pkgs.runCommand "kings-field-3-psyq-3.0-sdk" {
        nativeBuildInputs = with pkgs; [
          binutils
          coreutils
          file
          p7zip
          python3
          unar
        ];
      } ''
        export PSYQ_KIT_ZIP="${psyqKit}"
        python3 ${sdkBuilder} \
          --work-dir "$TMPDIR/toolchain-work" \
          --stage-dir "$out"
      '';

      # Sony's "GNU C Compiler Version 2.60 (World)" floppy (volume GNUCV260;
      # CC1PSX.EXZ/CPPPSX.EXZ dated 1994-10-07, gzip-packed, README 1994-10-19)
      # as preserved in Archive.org `ps1_sdks`. CC1PSX.EXE is "GNU C 2.6.0
      # [AL 1.1, MM 40] Sony Playstation", a go32 (DJGPP v1) DOS program run
      # under DOSBox-X. It is byte-identical to the CC1PSX.EXE of the mkst/esa
      # psyq3.3 and psyq3.5 bundles. The Psy-Q 3.0 kit's COMPILER/CC1PSX.EXE
      # is a different file with the same version string and, on every sample
      # tried, identical output. With -O2 -G8 it emits PSX.EXE's main
      # exactly, its __main call included.
      sonyGcc260Floppy = pkgs.fetchurl {
        name = "sony-gnu-c-2.60-disk1.img";
        url = "https://archive.org/download/ps1_sdks/GNU%20C%20Compiler%20Version%202.60%20%28World%29%20%28Disk%201%29.img";
        hash = "sha256-B6+f7NFIzUI0Ec65En7T8+kUmhFRoaJU3z4rjwwzuuc=";
      };
      sonyGcc260 = pkgs.runCommand "kings-field-3-sony-gnu-c-2.60" {
        nativeBuildInputs = with pkgs; [ coreutils gzip mtools ];
      } ''
        mkdir -p "$out"
        for tool in CC1PSX CPPPSX; do
          mcopy -n -i ${sonyGcc260Floppy} "::$tool.EXZ" "$TMPDIR/$tool.EXZ"
          gzip -dc < "$TMPDIR/$tool.EXZ" > "$out/$tool.EXE"
        done
        check() {
          test "$(sha256sum "$out/$1" | cut -d' ' -f1)" = "$2" || { echo "$1: unexpected hash" >&2; exit 1; }
        }
        check CC1PSX.EXE 6587ed37f9f8795f8e47601d3a71167e93c832959f2f314e08d11c53be04661e
        check CPPPSX.EXE 69e0b24ef219119200298ed450d5c8342529ee153f498a25c2166fea663c20d1
      '';

      # Sony's Programmer Tool Runtime Library 3.3 CD (DTL-S2190, files dated
      # 1995-10-20; redump in Archive.org `ps1_sdks`). Only data track 1 is
      # read, and only these files are copied, unchanged:
      #   lib/      PSX/LIB: the 3.3 archives, 2MBYTE.OBJ, NONE2.OBJ and
      #             MALLOC.OBJ (byte-identical to the Release 2.5 allocator);
      #   include/  PSX/INCLUDE, and include-lf/ the same with LF line endings;
      #   bin/      PSYQ/ASPSX.EXE (ASPSX 2.21, 1995-06-22), PSYQ/PSYLINK.EXE
      #             (PSYLINK 2.27, 1995-08-30), PSXGRAPH/BIN/CPE2X.EXE (CPE2X
      #             1.5, 1995-07-07) and PSX/BIN/DMPSX.EXE with its INLINE.TBL.
      # PSX.EXE rebuilds byte-identically from these tools and 2MBYTE.OBJ,
      # LIBAPI and LIBSN. The archives carry LIBETC intr.c 1.71 (END's) and
      # LIBGPU sys.c 1.107; GAME and OPEN link Library 3.4 (intr.c 1.73,
      # sys.c 1.116) and END a LIBGPU sys.c 1.109, none of which we found
      # preserved, so for those three programs these archives are a probe.
      psyq33Cd = pkgs.fetchurl {
        name = "psx-runtime-library-3.3-dtl-s2190-redump.zip";
        url = "https://archive.org/download/ps1_sdks/Programmer%20Tool%20-%20Runtime%20Library%20Version%203.3%20%28Japan%29_DTL-S2190_redump.zip";
        hash = "sha256-Vba1f7N6O6DCuNepSlUcp2RF+93DlIzz9SfFFfPP9W4=";
      };
      psyq33Sdk = pkgs.runCommand "kings-field-3-psyq-runtime-3.3" {
        nativeBuildInputs = with pkgs; [ coreutils findutils python3 unzip ];
      } ''
        cd "$TMPDIR"
        track='PlayStation Programmer Tool - Runtime Library Version 3.3 (Japan) (Track 1).bin'
        unzip -q ${psyq33Cd} "$track"
        extract() { python3 ${./extract-iso9660.py} "$track" "$@"; }
        extract "$out/lib" PSX/LIB/
        extract "$out/include" PSX/INCLUDE/
        extract "$out/include/SYS" PSX/INCLUDE/SYS/
        extract "$out/bin" PSYQ/ASPSX.EXE PSYQ/PSYLINK.EXE PSXGRAPH/BIN/CPE2X.EXE \
          PSX/BIN/DMPSX.EXE PSX/BIN/INLINE.TBL
        check() {
          test "$(sha256sum "$out/$1" | cut -d' ' -f1)" = "$2" || { echo "$1: unexpected hash" >&2; exit 1; }
        }
        check bin/ASPSX.EXE b52bf3d40a516333e679f18e9297883158e3015e8d97ac501e108b049617ce41
        check bin/PSYLINK.EXE 39c9e16ba84ddfb366004dc36f9c0b5a30f9908e56f8fca4b06ddb6eb748243b
        check bin/CPE2X.EXE 777caca88327cd18d4f92b43439575c1759381690d49f14177805a94b8ecbef6
        check bin/DMPSX.EXE c548939dce634074dd4f8e686514d32b8d870e6e9f65808c46cf10d503602176
        check bin/INLINE.TBL 2d869c07c00f718f34e63c57102a97ff45d7d305332f15aa7ad99963ea3ff913
        check lib/2MBYTE.OBJ b4ed8b2e76bc841f7a81132906d846877aefa8f6233fb738600c8b1ebd032a46
        check lib/NONE2.OBJ 38662381b57cdbebb8d083842fd23914bad1d4c66303beb871f2f5248c38774c
        check lib/MALLOC.OBJ 628e405fd0e3acfff2ce9d4a15d481f0aa36398c14e9eae0a82b7ff0a86a74c9
        check lib/LIBAPI.LIB c86ad75d077094ba51acbb04129a37d98682a79d241d1cf458a649df600eaa10
        check lib/LIBC.LIB 35632724889f04dd81fa5a8460e5d373cbd90c9f00a4e3b8a85a4626dcbda432
        check lib/LIBCARD.LIB 2a647d21c158d81422eb48917d513850549087e737188451343da43d2eba995d
        check lib/LIBCD.LIB 122fe1696237fa2dd18bace4d12f0d49658c597fc25f57bc81295cb24aa69242
        check lib/LIBETC.LIB 23435518b40906e1f0ff34219edb7873cb69c08bb6b75f0fd9b1713bc1b3dd19
        check lib/LIBGPU.LIB 01e301fd645c47c800a4d13d44a40807f8e9e01bbe97ee91bd706f3a816f9f60
        check lib/LIBGTE.LIB 0f8bd56b89d1ab71d7fb4352ab4d22eedf8a9bf8ab2315ea13479c2606b24203
        check lib/LIBPRESS.LIB 7ed2866b5e2d3d855936f9fe5c3c330dd3a360c3ab9d49e2bff107a500b42d6c
        check lib/LIBSN.LIB 1ef99108b065166613a4bf6c5b46a74bd0f584e8a570926e8e0c66497e64f547
        check lib/LIBSND.LIB ecb60248990ae397a1f55acab1cf214ebc4f5acc9e7413c07d89fcf22b826648
        check lib/LIBSPU.LIB 8c75abf4934f30a707b389c7e5ef7a45dec7e8b60a0a1ae6dad02918ff5a2185
        (cd "$out/include" && find . -type f) | while read -r header; do
          mkdir -p "$out/include-lf/$(dirname "$header")"
          tr -d '\r' < "$out/include/$header" > "$out/include-lf/$header"
        done
      '';

      cc1psx260 = pkgs.writeShellApplication {
        name = "cc1psx-260";
        text = ''exec ${gcc260Native}/bin/cc1 "$@"'';
      };

      cpppsx260 = pkgs.writeShellApplication {
        name = "cpppsx-260";
        text = ''exec ${gcc260Native}/bin/cpp "$@"'';
      };

      cc1psx257 = pkgs.writeShellApplication {
        name = "cc1psx-257";
        text = ''exec ${gcc257Native}/bin/cc1 "$@"'';
      };

      cpppsx257 = pkgs.writeShellApplication {
        name = "cpppsx-257";
        text = ''exec ${gcc257Native}/bin/cpp "$@"'';
      };

in {
  inherit psyqSdk psyq33Sdk sonyGcc260 gcc257Native gcc257Headers gcc260Native
    cc1psx257 cpppsx257 cc1psx260 cpppsx260;
}
