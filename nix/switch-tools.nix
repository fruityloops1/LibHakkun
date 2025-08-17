{
  stdenv,
  fetchFromGitHub,
  lib,
  autoreconfHook,
  pkg-config,
  lz4,
  zlib,
  ...
}:
stdenv.mkDerivation rec {
  pname = "switch-tools";
  version = "v1.13.1";

  src = fetchFromGitHub {
    owner = "switchbrew";
    repo = pname;
    rev = version;
    hash = "sha256-WI8sHucTeZOCQWlVdv5fFHK1ENdajUVXlvaW1lJfSMc=";
  };

  nativeBuildInputs = [
    autoreconfHook
    pkg-config
  ];

  buildInputs = [
    lz4
    zlib
  ];
}
