{
  system,
  mkShell,
  openssl,
  meson,
  tree,
  ninja,
  doxygen,
  pkg-config,
  gtest,
  self,
}: let
  inherit (self.checks.${system}) pre-commit-check;
in
  mkShell {
    packages = [
      openssl
      meson
      ninja
      tree
      gtest
      doxygen
      pkg-config
    ];

    shellHook =
      pre-commit-check.shellHook
      + ''
        echo "Development Shell ready"
      '';
  }
