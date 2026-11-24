{
  description = "King's Field III (SLUS-00255) source";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/64c08a7ca051951c8eae34e3e3cb1e202fe36786";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; config.allowUnfree = true; };
      sdk = import ./nix/psx-toolchain.nix {
        inherit pkgs;
        sdkBuilder = ./scripts/create-toolchain.py;
      };
      emulator = import ./nix/pcsx-redux.nix { inherit pkgs; };
      environment = {
        PSYQ_C_INCLUDE = "${sdk.gcc257Headers}/include";
        PSYQ_SDK = "${sdk.psyqSdk}/psyq-3.0";
        PSYQ_BIN = "${sdk.psyqSdk}/psyq-3.0/BIN";
        PSYQ_INCLUDE = "${sdk.psyq33Sdk}/include-lf";
        PSYQ_LIB = "${sdk.psyq33Sdk}/lib";
        PSYQ_MALLOC_OBJ = "${sdk.psyq33Sdk}/lib/MALLOC.OBJ";
        PSYQ_CC1PSX = "${sdk.sonyGcc260}/CC1PSX.EXE";
        PSYQ_ASPSX = "${sdk.psyq33Sdk}/bin/ASPSX.EXE";
        PSYQ_PSYLINK = "${sdk.psyq33Sdk}/bin/PSYLINK.EXE";
        PSYQ_CPE2X = "${sdk.psyq33Sdk}/bin/CPE2X.EXE";
        PSYQ_DMPSX = "${sdk.psyq33Sdk}/bin/DMPSX.EXE";
      };
      buildTools = [ pkgs.python3 pkgs.dosbox-x sdk.cpppsx260 sdk.cc1psx260 sdk.cpppsx257 sdk.cc1psx257 ];
      game = pkgs.stdenvNoCC.mkDerivation (environment // {
        pname = "kings-field";
        version = "1.0";
        src = pkgs.lib.cleanSourceWith {
          src = ./.;
          filter = path: type:
            let name = baseNameOf path;
            in !(builtins.elem name [ ".git" "build" "result" "__pycache__" "codecs" ]);
        };
        nativeBuildInputs = buildTools;
        dontConfigure = true;
        buildPhase = ''
          runHook preBuild
          python3 build.py
          runHook postBuild
        '';
        installPhase = ''
          runHook preInstall
          mkdir -p "$out"
          cp build/psx/PSX.EXE build/game/GAME.EXE build/open/OPEN.EXE build/end/END.EXE "$out/"
          mkdir -p "$out/link"
          cp build/psx/PSX.CPE build/game/GAME.CPE build/open/OPEN.CPE build/end/END.CPE "$out/link/"
          runHook postInstall
        '';
      });
      runner = pkgs.writeShellApplication {
        name = "kings-field";
        runtimeInputs = [ pkgs.python3 emulator ];
        text = ''
          export PCSX_REDUX_BIN="${emulator}/bin/pcsx-redux"
          export PCSX_REDUX_BIOS="${emulator.openbios}"
          exec python3 ${./scripts/psxbuild/disc.py} --executables ${game} "$@"
        '';
      };
      retailRunner = pkgs.writeShellApplication {
        name = "kf-run-retail";
        runtimeInputs = [ pkgs.python3 emulator ];
        text = ''
          export PCSX_REDUX_BIN="${emulator}/bin/pcsx-redux"
          export PCSX_REDUX_BIOS="${emulator.openbios}"
          exec python3 ${./scripts/psxbuild/disc.py} --retail "$@"
        '';
      };
    in {
      packages.${system} = { inherit game; default = game; };
      apps.${system} = {
        default = { type = "app"; program = "${runner}/bin/kings-field"; };
        retail = { type = "app"; program = "${retailRunner}/bin/kf-run-retail"; };
      };
      devShells.${system} = {
        default = pkgs.mkShell (environment // { packages = buildTools ++ [ retailRunner ]; });
        codecs = pkgs.mkShell { packages = [ pkgs.cargo pkgs.rustc ]; };
      };
    };
}
