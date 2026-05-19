pkgs:
with pkgs;
let
  llvm = llvmPackages_20;
in
rec {
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
      llvm.bintools-unwrapped
      lld
      curl
      cmake
      ninja
      pkg-config
      nixfmt
    ];

  devShell =
    hakkunLocation:
    mkShellNoCC.override
      {
        stdenv = pkgs.stdenvNoCC.override {
          # no more <arm_neon.h> include errors
          hasCC = true;
          cc = wrapCCWith {
            cc = llvm.clang-unwrapped;
            gccForLibs = null;
            extraPackages = [ llvm.compiler-rt ];
            extraBuildCommands = ''
              rsrc="$out/resource-root"
              mkdir "$rsrc"
              echo "-resource-dir=$rsrc" >> $out/nix-support/cc-cflags
              echo "export NIX_CC_WRAPPER_SUPPRESS_TARGET_WARNING=1" >> $out/nix-support/setup-hook
              ln -s "${llvm.clang-unwrapped.lib}/lib/clang/20/include" "$rsrc"
              ln -s "${llvm.compiler-rt.out}/lib" "$rsrc/lib"
              ln -s "${llvm.compiler-rt.out}/share" "$rsrc/share"
            '';
          };
          allowedRequisites = null;
        };
      }
      rec {
        nativeBuildInputs = dependencies ++ [ build-sail ];
        LD_LIBRARY_PATH = lib.makeLibraryPath nativeBuildInputs;
        hardeningDisable = [ "all" ];

        shellHook = ''
          ${build-sail}/bin/build-sail ${hakkunLocation} > /dev/null
        '';
      };

  build-sail = stdenv.mkDerivation {
    name = "build-sail";
    dontUnpack = true;
    nativeBuildInputs = [ makeWrapper ];
    installPhase = ''
      makeWrapper ${writeShellScript "build-sail-inner" ''
        cmake -S $1/sail -B $1/sail/build
        cmake --build $1/sail/build --parallel 
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
