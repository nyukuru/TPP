{
  stdenv,
  meson,
  ninja,
  pkg-config,
  gtest,
  lib,
  openssl,
}:
stdenv.mkDerivation (finalAttrs: {
  pname = "tpp";
  version = "0.1.0";

  src = ../.;

  doCheck = true;

  mesonFlags = [
    (lib.mesonBool "enable-tests" finalAttrs.finalPackage.doCheck)
  ];

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];

  buildInputs = [
    openssl
  ];

  checkInputs = [
    gtest
  ];
})
