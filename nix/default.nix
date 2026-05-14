pkgs: with pkgs; rec {
  dependencies =
    let
      python3 = pkgs.python3.withPackages (pypkgs: [
        pypkgs.pyelftools
        pypkgs.mmh3
        pypkgs.lz4
      ]);
    in
    [
      python3
      llvmPackages.bintools-unwrapped
      lld
      curl
      cmake
      ninja
      pkg-config
      nixfmt
    ];

  devShell =
    mkShellNoCC.override
      {
        stdenv = pkgsCross.aarch64-embedded.llvmPackages_20.stdenv;
      }
      rec {
        nativeBuildInputs = dependencies ++ [ build-sail ];
        LD_LIBRARY_PATH = lib.makeLibraryPath (
          nativeBuildInputs
          ++ [
            llvmPackages_20.clang-unwrapped.lib
          ]
        );

        shellHook = ''
          ${build-sail}/bin/build-sail > /dev/null
        '';
      };

  build-sail = stdenv.mkDerivation {
    name = "build-sail";
    dontUnpack = true;
    nativeBuildInputs = [ makeWrapper ];
    installPhase = ''
      makeWrapper ${writeShellScript "build-sail-inner" ''
        cmake -S sys/hakkun/sail -B sys/hakkun/sail/build
        cmake --build sys/hakkun/sail/build --parallel
      ''} $out/bin/build-sail \
        --prefix PATH : ${
          lib.makeBinPath [
            cmake
            gnumake
            llvmPackages_20.clang
          ]
        }
    '';
  };
}
