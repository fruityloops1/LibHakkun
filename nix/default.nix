pkgs:
with pkgs; rec {
  localPackages = {
    switch-tools = pkgs.callPackage ./switch-tools.nix {};
  };
  dependencies = let
    python3 = pkgs.python3.withPackages (pypkgs: [
      pypkgs.pyelftools
      pypkgs.mmh3
      pypkgs.lz4
    ]);
  in [
    localPackages.switch-tools
    python3
    llvmPackages.clangUseLlvm
    llvmPackages.bintools-unwrapped
    lld
    curl
    cmake
    ninja
  ];
}
