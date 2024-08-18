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
      rev = with self; if sourceInfo?dirtyRev then sourceInfo.dirtyRev else sourceInfo.rev;
      pkgs = import nixpkgs { inherit system; };
      n64_inst = (n64-tools.packages.${system}.mkLibDragon {
        rev = "f3aae88520fd9427c969961b556d1bccdb5c89de";
        hash = "sha256-/yigAAPQWAZg5x7QbiHxdsd+PxuLLfD8oJFAweU0MoI=";
      }).n64_inst;
    in
    {
        devShells.default = pkgs.mkShell
        {
          buildInputs = with pkgs; [
            cmake
            ninja
          ];
          N64_INST = n64_inst;
        };
    }
  );
}
