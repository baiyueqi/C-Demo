{
  description = "The foundation of wonderland C++ infra.";

  inputs = {
    # Pointing to the current stable release of nixpkgs. You can
    # customize this to point to an older version or unstable if you
    # like everything shining.
    #
    # E.g.
    #
    # nixpkgs.url = "github:NixOS/nixpkgs/unstable";
    nixpkgs.url = "github:NixOS/nixpkgs/24.11";
    utils.url = "github:numtide/flake-utils";

    vitalpkgs.url = "github:nixvital/vitalpkgs";
    vitalpkgs.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = { self, nixpkgs, vitalpkgs, ... }@inputs: inputs.utils.lib.eachSystem [
    # Add the system/architecture you would like to support here. Note that not
    # all packages in the official nixpkgs support all platforms.
    "x86_64-linux" "i686-linux" "aarch64-linux" "x86_64-darwin"
  ] (system: let pkgs = import nixpkgs {
                   inherit system;

                   # Add overlays here if you need to override the nixpkgs
                   # official packages.
                    overlays = [
                      vitalpkgs.overlays.default
                    ];
                     
                   # Uncomment this if you need unfree software (e.g. cuda) for
                   # your project.
                   #
                   # config.allowUnfree = true;
                    config.permittedInsecurePackages = [
                      # *Exceptionally*, those packages will be cached with their *secure* dependents
                      # because they will reach EOL in the middle of the 23.05 release
                      # and it will be too much painful for our users to recompile them
                      # for no real reason.
                      # Remove them for 23.11.
                      "nodejs-16.20.1"
                      "openssl-1.1.1u"
                    ];
                 };
             in {
               devShells.default = pkgs.mkShell.override {
                 stdenv = pkgs.llvmPackages_16.stdenv;
               } rec {
                 # Update the name to something that suites your project.
                 name = "basis";

                 packages = with pkgs; [
                   # Development Tools
                   cmake

                   # Development time dependencies
                  #  gtest

                   # Build time and Run time dependencies
                   llvmPackages_16.clang
                  (boost.override { enablePython = true; python = pkgs.python3; extraB2Args = [ " --with-locale stage " ]; })
                  #  marl
                  spdlog
                  #  abseil-cpp
                  nlohmann_json
                  vscode-include-fix
                  boost
                  #  drogon
                 ];

                 # Setting up the environment variables you need during
                 # development.
                 shellHook = let
                   icon = "f121";
                 in ''
                    export PS1="$(echo -e '\u${icon}') {\[$(tput sgr0)\]\[\033[38;5;228m\]\w\[$(tput sgr0)\]\[\033[38;5;15m\]} (${name}) \\$ \[$(tput sgr0)\]"
                 '';
               };

             });
}