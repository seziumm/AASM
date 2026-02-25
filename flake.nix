{
  description = "AASM dev shells (host + RISC-V 32-bit)";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };

      # Toolchain cross RISC-V 32 bit (bare-metal), basato sugli esempi di nixpkgs.
      riscv32-pkgs = import nixpkgs {
        inherit system;
        crossSystem = nixpkgs.lib.systems.examples.riscv32-embedded;
      };
    in
    {
      devShells.${system} = {
        # Shell di sviluppo host (LLVM, CMake, ecc.) usata per AASM.
        default = pkgs.mkShell {
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

        # Shell per toolchain RISC-V 32 bit + QEMU + GDB.
        #
        # Uso:
        #   nix develop .#riscv32   (o: nix-shell --attr devShells.x86_64-linux.riscv32)
        #   riscv32-none-elf-gcc    # compilatore cross
        #   riscv32-none-elf-gdb    # gdb cross bare-metal
        #   qemu-system-riscv32     # per eseguire bin/*.elf prodotti da AASM
        riscv32 = riscv32-pkgs.mkShell {
          nativeBuildInputs = [
            riscv32-pkgs.buildPackages.gcc
            riscv32-pkgs.buildPackages.gdb
            pkgs.qemu
          ];
        };
      };
    };
}
