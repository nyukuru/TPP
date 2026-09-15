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
  nlohmann_json,
  claude-code,
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
      nlohmann_json
      doxygen
      pkg-config
      claude-code
    ];

    shellHook =
      pre-commit-check.shellHook
      + ''
        echo "Development Shell ready"
      '';
  }
