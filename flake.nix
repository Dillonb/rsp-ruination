{
  description = "N64 RSP Torture Test";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    n64-tools.url = "github:Dillonb/n64-tools.nix";
  };

  outputs = { self, nixpkgs, flake-utils, n64-tools }: flake-utils.lib.eachDefaultSystem (system:
    let
      shortRev = with self; if sourceInfo?dirtyShortRev then sourceInfo.dirtyShortRev else sourceInfo.shortRev;
      pkgs = import nixpkgs { inherit system; };
      n64_inst = (n64-tools.packages.${system}.mkLibDragon {
        rev = "f3aae88520fd9427c969961b556d1bccdb5c89de";
        hash = "sha256-/yigAAPQWAZg5x7QbiHxdsd+PxuLLfD8oJFAweU0MoI=";
      }).n64_inst;
    in
    {
        packages.default = pkgs.stdenv.mkDerivation
        {
          pname = "rsp-ruination";
          version = shortRev;
          src = ./.;
          N64_INST = n64_inst;
          installPhase = "install -Dm644 rsp-ruination.z64 $out/rsp-ruination.z64";
        };

        devShells.default = pkgs.mkShell
        {
          N64_INST = n64_inst;
        };
    }
  );
}
