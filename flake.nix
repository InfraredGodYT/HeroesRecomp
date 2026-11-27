# devShell only; for copyright reasons, as vendoring default.xbe for building
#                in a Nix flake is tricky - Sewer
#
# This shell is for development only- does not build binaries that are 'purely'
# linked via Nix. If there's any flake for end users etc. it'd be something like
# a wrapped AppImage; due to the
# above limitation.
{
  description = "Heroes Recompiled - unofficial PC port of Sonic Heroes (2003, Xbox) via static recompilation (dev shell)";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = {
    self,
    nixpkgs,
  }: let
    system = "x86_64-linux"; # only x86 is tested
    pkgs = nixpkgs.legacyPackages.${system};
    lib = nixpkgs.lib;

    # SDL3 static build deps, exported via CMake find modules / pkg-config.
    sdlDeps = with pkgs; [
      # SDL - X11
      libX11 # also linked directly: find_package(X11) in HeroesRecomp/CMakeLists
      libXcursor
      libXext
      libXfixes
      libXi
      libXrandr
      libXScrnSaver
      libXtst

      # SDL - X11 session management
      libSM
      libICE

      # SDL - Wayland
      wayland
      libdecor
      libxkbcommon

      # SDL - audio
      alsa-lib
      libpulseaudio
      pipewire

      # SDL - input
      udev # libudev: evdev + HIDAPI
    ];

    # Runtime libs for the prebuilt dxc-linux / libdxcompiler.so (FHS binaries).
    dxcRuntimeLibs = lib.makeLibraryPath [
      pkgs.zlib
      pkgs.libgcc
      pkgs.gcc-unwrapped.lib # libstdc++
      pkgs.glibc
    ];

    # Runtime libs for the game binary:
    #  - vulkan-loader: dlopen'd by volk
    #  - sdlDeps: SDL3 dlopen's its platform backends
    #  - rest: shared deps missing from the binary's RUNPATH (pinned to $ORIGIN)
    gameRuntimeLibs = lib.makeLibraryPath (
      [
        pkgs.vulkan-loader
        pkgs.freetype
        pkgs.nativefiledialog-extended
        pkgs.curl
        pkgs.dbus
        pkgs.gcc-unwrapped.lib
        pkgs.libgcc
        pkgs.zlib
      ]
      ++ sdlDeps
    );

    # Build-time rpath: XboxRecomp runs during the build.
    buildRpath = lib.makeLibraryPath [
      pkgs.gcc-unwrapped.lib
      pkgs.glibc
      pkgs.libgcc
    ];

    # Invokes the prebuilt dxc-linux through its loader instead of patchelfing
    # a copy: no intermediate binary, repo stays untouched.
    # CMake ;-list: expands to separate COMMAND arguments.
    dxcTool = let
      loader = "${pkgs.glibc}/lib/ld-linux-x86-64.so.2";
      dxcLibs = "$SR_ROOT/tools/XShaderRecomp/thirdparty/dxc-bin/lib/x64";
      dxcBin = "$SR_ROOT/tools/XShaderRecomp/thirdparty/dxc-bin/bin/x64/dxc-linux";
    in "${loader};--library-path;${dxcRuntimeLibs}:${dxcLibs};${dxcBin}";
  in {
    devShells.${system}.default = pkgs.clangStdenv.mkDerivation {
      name = "heroesrecomp-devshell";

      # Values consumed by nix/sr-env.sh (see shellHook).
      SR_DXC = dxcTool;
      SR_BUILD_RPATH = buildRpath;
      SR_GAME_LIBS = gameRuntimeLibs;
      SR_LLVM_AR = "${pkgs.libllvm}/bin/llvm-ar";
      SR_LLVM_RANLIB = "${pkgs.libllvm}/bin/llvm-ranlib";
      SR_NLOHMANN_JSON_SRC = "${pkgs.nlohmann_json.src}";

      nativeBuildInputs = with pkgs; [
        cmake
        ninja
        cargo
        rustc
        pkg-config
        clang
        clang-tools # clangd: IDE language server
        lld
        libllvm # llvm-ar / llvm-ranlib: link-time optimization archives
        git # version.cmake: find_package(Git)
        wayland-scanner # SDL3 wayland protocols
        gdb
        vulkan-tools
      ];

      buildInputs = with pkgs;
        [
          curl # find_package(CURL)
          freetype
          tinyxml-2 # msdfgen SVG
          nativefiledialog-extended # find_package(nfd)
          gtk3
          libX11 # find_package(X11)
          vulkan-headers
          vulkan-loader
          zlib
          libgcc
          gcc-unwrapped.lib
        ]
        ++ sdlDeps;

      # Project is non-PIE (recomp assumptions).
      # zerocallusedregs: clears registers before every return; bloats recompiled code.
      hardeningDisable = ["pic" "zerocallusedregs"];

      # Portable script from nix/ does the actual work.
      shellHook = ''
        export SR_ROOT="$PWD"
        # clangStdenv's `clang` package puts an 'unwrapped' toolset
        # first on PATH. Its clangd can't find nix stdlib headers.
        # Prepend wrapped clangd so VSCode and friends pick that one.
        export PATH="${pkgs.clang-tools}/bin:$PATH"

        # Vulkan ICD discovery.
        export XDG_DATA_DIRS="/run/opengl-driver/share''${XDG_DATA_DIRS:+:$XDG_DATA_DIRS}"
        . "$SR_ROOT/nix/sr-env.sh"
      '';
    };
  };
}
