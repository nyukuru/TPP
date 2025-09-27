{
  description = "C++ Twitch API Library, inspired by DPP";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.pre-commit-hooks.url = "github:cachix/git-hooks.nix";

  outputs = {
    self,
    nixpkgs,
    pre-commit-hooks,
    ...
  }: let
    # Supported systems, non-exhaustive as of now
    supportedSystems = [
      "x86_64-linux"
      "x86_64-darwin"
      "aarch64-linux"
      "aarch64-darwin"
    ];

    forAllSystems = func:
      nixpkgs.lib.genAttrs
      supportedSystems
      (system: func system nixpkgs.legacyPackages.${system});
  in {
    overlays.default = final: prev: self.packages.${final.system};

    packages = forAllSystems (
      system: pkgs:
        (pkgs.lib.packagesFromDirectoryRecursive {
          directory = ./nix-packages;
          callPackage = pkgs.newScope (
            self.packages.${system}
            // {self = builtins.removeAttrs self ["packages"];}
          );
        })
        // {default = self.packages.${system}.tpp;}
    );

    devShells = forAllSystems (system: pkgs: {
      default = self.packages.${system}.dev-shell;
    });

    checks = forAllSystems (system: pkgs: {
      pre-commit-check = pre-commit-hooks.lib.${system}.run {
        src = ./.;
        hooks = {
          alejandra.enable = true;
          clang-format.enable = true;
          clang-format.types_or = pkgs.lib.mkForce ["c" "c++"];
        };
      };
    });
  };
}
