{
  stdenv,
  cmake,
  openssl,
}:
stdenv.mkDerivation {
  pname = "tpp";
  version = "0.0.1b";

  src = ../.;

  buildInputs = [openssl];
  nativeBuildInputs = [cmake];
}
