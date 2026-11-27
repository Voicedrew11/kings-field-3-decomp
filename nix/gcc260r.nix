{ pkgs }:
# GNU C 2.6.0 for the PSX target, built from the FSF source with Decompals'
# old-gcc `gcc-2.6.0-psx` recipe (the recipe of the flake's cc1psx-260), and
# the same compiler with FSF's reload change of 1994-08-07
# (patches/gcc-2.6.0-reload-19940807.patch). The patched compiler is a
# reconstruction: no preserved Sony binary carries that change. See
# docs/toolchain.md, "GAME's compiler revision".
let
  recipeRevision = "b74211c9d959e9724802f3177c8229cd67202c87";
  recipeArchive = pkgs.fetchurl {
    name = "old-gcc-${recipeRevision}.tar.gz";
    url = "https://codeload.github.com/decompals/old-gcc/tar.gz/${recipeRevision}";
    hash = "sha256-De+MYfwfyABo2JUvsIWjxNu0Bp8fNi1wxpiWn8O5bMI=";
  };
  # The FSF 2.6.0 release (1994-07-14) from the mirror the 2.5.7 source and
  # the Decompals recipe use.
  gccArchive = pkgs.fetchurl {
    url = "https://www.nic.funet.fi/index/gnu/funet/historical-funet-gnu-area-from-early-1990s/gcc-2.6.0.tar.gz";
    hash = "sha256-Z4shcJjEDqI8FP82xCxuHIWjdbHZ+iNVJfIJyCZSVpU=";
  };
  # gcc-2.6.0-psx.Dockerfile, step for step.
  source = pkgs.runCommand "gcc-2.6.0-psx-source-${recipeRevision}" {
    nativeBuildInputs = [ pkgs.gnutar pkgs.gzip pkgs.patch ];
  } ''
    mkdir recipe "$out"
    tar -xzf ${recipeArchive} -C recipe --strip-components=1
    tar -xzf ${gccArchive} -C "$out" --strip-components=1
    chmod -R u+w "$out"
    cd "$out"
    sed -i 's/include <varargs.h>/include <stdarg.h>/g' *.c
    for pair in \
      obstack.h:obstack-2.7.2.h.patch \
      sdbout.c:sdbout-2.6.0.c.patch \
      collect2.c:collect2-2.6.0.c.patch \
      cccp.c:cccp-2.6.0.c.patch \
      gcc.c:gcc-2.6.0.c.patch \
      cp/g++.c:g++-2.6.0.c.patch \
      config/mips/mips.h:mipsel-2.6.patch; do
      patch -u -p1 "''${pair%%:*}" -i "$NIX_BUILD_TOP/recipe/patches/''${pair#*:}"
    done
    patch -s -u -p1 < "$NIX_BUILD_TOP/recipe/patches/psx-2.5.7.patch"
  '';
  mkCompiler = { name, patches }: pkgs.pkgsi686Linux.stdenv.mkDerivation {
    pname = name;
    version = "2.6.0-${recipeRevision}";
    src = source;
    inherit patches;
    dontUpdateAutotoolsGnuConfigScripts = true;
    hardeningDisable = [ "all" ];
    dontAddStaticConfigureFlags = true;
    configurePhase = ''
      runHook preConfigure
      ./configure --target=mips-sony-psx --prefix="$out" \
        --with-endian-little --with-gnu-as \
        --host=i386-pc-linux --build=i386-pc-linux
      runHook postConfigure
    '';
    # The recipe's flags (no -O: the 1994 sources are built unoptimised), plus
    # what a current host GCC needs to accept K&R code.
    buildPhase = ''
      runHook preBuild
      host_flags="-std=gnu89 -m32 -fcommon -Dbsd4_4 -Dmips -DHAVE_STRERROR -Wno-error=implicit-function-declaration -Wno-error=implicit-int -Wno-error=incompatible-pointer-types -Wno-error=int-conversion"
      make -j "$NIX_BUILD_CORES" insn-config.h insn-codes.h insn-flags.h insn-attr.h CFLAGS="$host_flags"
      make -j "$NIX_BUILD_CORES" cc1 CFLAGS="$host_flags"
      runHook postBuild
    '';
    doCheck = true;
    checkPhase = ''
      ./cc1 -version -quiet < /dev/null -o /dev/null 2>&1 | grep -F 'GNU C version 2.6.0 [AL 1.1, MM 40] Sony Playstation'
    '';
    installPhase = ''
      runHook preInstall
      mkdir -p "$out/bin"
      cp cc1 "$out/bin/${name}"
      runHook postInstall
    '';
  };
in {
  inherit source recipeRevision;
  # Control: must equal cc1psx-260 (the Decompals 0.17 binary) on every unit.
  plain = mkCompiler { name = "cc1psx-260-source"; patches = [ ]; };
  reload = mkCompiler { name = "cc1psx-260r"; patches = [ ../patches/gcc-2.6.0-reload-19940807.patch ]; };
}
