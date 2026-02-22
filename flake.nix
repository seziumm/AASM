{
  description = "A";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        inherit (pkgs.llvmPackages_19) stdenv;
        buildInputs = [
          pkgs.llvmPackages_latest.lldb
          pkgs.llvmPackages_latest.libllvm
          pkgs.llvmPackages_latest.libcxx
          pkgs.llvmPackages_latest.clang
          pkgs.clang-tools
          pkgs.clang
          pkgs.cmake
        ];

      };
    };
}
