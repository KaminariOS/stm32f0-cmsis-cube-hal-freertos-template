{
  description = "Realtime Vulkan ray tracing";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable"; 
    rust-overlay.url = "github:oxalica/rust-overlay";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, flake-utils, rust-overlay,... }: let
    lib = {
      inherit (flake-utils.lib) defaultSystems eachSystem;
    };
    supportedSystems = [ "x86_64-linux" ];
  in lib.eachSystem supportedSystems (system: let
    nightlyVersion = "2023-12-15";

    #pkgs = mars-std.legacyPackages.${system};
    pkgs = import nixpkgs {
        inherit system;
        overlays = [
          (import rust-overlay)
          #(import ./pkgs)
        ];
      };
    pinnedRust = pkgs.rust-bin.nightly.${nightlyVersion}.default.override {
      extensions = ["rustc-dev" "rust-src" "rust-analyzer-preview" ];
      targets = [ "thumbv6m-none-eabi" ];
    };
    rustPlatform = pkgs.makeRustPlatform {
      rustc = pinnedRust;
      cargo = pinnedRust;
    };
    cargoExpand = pkgs.cargo-expand.override { inherit rustPlatform; };
  in {
    
devShell = pkgs.mkShell rec {
  hardeningDisable = [
    "fortify"
  ];
  nativeBuildInputs = [
    # pinnedRust 
    # cargoExpand
      ] ++ (with pkgs; [
        gcc-arm-embedded-13
        gdb 
        openocd
        bear
        # cargo-generate
        # usbutils
      ]);
  buildInputs = with pkgs; [
#    alsaLib
#    binaryen
#    fontconfig
#    freetype
#    libxkbcommon
#    spirv-tools
    #udev

#    xorg.libXi
#    xorg.libXrandr
#    gcc-unwrapped.lib
  ];


  shellHook = ''
  '';
};

  });
}
