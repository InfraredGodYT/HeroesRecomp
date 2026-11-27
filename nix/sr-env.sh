# HeroesRecomp dev shell: setup + commands, in one file.
#
# Sourced by the flake's shellHook. Lifted out for cleanliness; to avoid
# a nasty looking indent.
#
# Flake-provided env (SR_ROOT is set by the flake's shellHook):
#  - SR_DXC: dxc-linux invocation through its loader (CMake ;-list)
#  - SR_BUILD_RPATH: rpath for build-time tools (XboxRecomp runs in-build)
#  - SR_GAME_LIBS: LD_LIBRARY_PATH for the game binary (global, see below)
#  - SR_LLVM_AR / SR_LLVM_RANLIB: archive tools for link-time optimization
#  - SR_NLOHMANN_JSON_SRC: pinned nlohmann_json source (FetchContent)

# ------------------------------------------------------------------------------
# Environment
# ------------------------------------------------------------------------------

if [ -z "${SR_BUILD_RPATH:-}" ]; then
  echo "heroesrecomp: dev-shell environment missing; enter the nix dev shell first" >&2
  return 1
fi

# The flake bakes $SR_ROOT as a placeholder; expand it here (CMake/ninja won't).
export SR_DXC="${SR_DXC//\$SR_ROOT/$SR_ROOT}"

# Runtime libs for the game exe + prebuilt deps (libdxcompiler.so).
export LD_LIBRARY_PATH="${SR_GAME_LIBS}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# ------------------------------------------------------------------------------
# Commands
# ------------------------------------------------------------------------------

# Build directory for a CMake build type, e.g. Debug -> out/build/nix-debug.
sr_type_dir() { echo "$SR_ROOT/out/build/nix-$(echo "$1" | tr '[:upper:]' '[:lower:]')"; }

# CMake configure
sr_configure() {
  local type="$1"; shift
  if [ ! -f "$SR_ROOT/tools/XShaderRecomp/thirdparty/dxc-bin/bin/x64/dxc-linux" ]; then
    echo "heroesrecomp: dxc-linux missing (submodule initialization failed?)" >&2
    return 1
  fi
  # Release-family builds get LTO everywhere else (see CMakePresets.json); match
  # that here regardless of which entry point (configure-*/build-*) runs.
  if [ "$type" = "Release" ] || [ "$type" = "RelWithDebInfo" ]; then
    set -- "$@" -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
  fi
    cmake -S "$SR_ROOT" -B "$(sr_type_dir "$type")" -G Ninja \
    -DCMAKE_BUILD_TYPE="$type" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_LINKER_TYPE=LLD \
    -DCMAKE_POSITION_INDEPENDENT_CODE=OFF \
    -DCMAKE_BUILD_RPATH_USE_ORIGIN=ON \
    -DCMAKE_BUILD_RPATH="$SR_BUILD_RPATH" \
    -DCMAKE_C_COMPILER_AR="$SR_LLVM_AR" \
    -DCMAKE_C_COMPILER_RANLIB="$SR_LLVM_RANLIB" \
    -DCMAKE_CXX_COMPILER_AR="$SR_LLVM_AR" \
    -DCMAKE_CXX_COMPILER_RANLIB="$SR_LLVM_RANLIB" \
    -DFETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON="$SR_NLOHMANN_JSON_SRC" \
    -DDIRECTX_DXC_TOOL="$SR_DXC" \
    -DSDL_VULKAN=ON -DSDL_X11=ON -DSDL_WAYLAND=ON \
    -DSDL_KMSDRM=OFF -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF \
    -DSDL_JACK=OFF -DSDL_OSS=OFF -DSDL_DISKAUDIO=OFF \
    -DSDL_DBUS=OFF -DSDL_IBUS=OFF -DSDL_FCITX=OFF \
    -DSDL_RPATH=OFF -DSDL_TESTS=OFF \
    -DMSDFGEN_USE_OPENMP=OFF \
    "$@" || return 1

  # Clangd (VSCode etc.): point at the freshest compile database via a root symlink.
  ln -sfn "$(sr_type_dir "$type")/compile_commands.json" "$SR_ROOT/compile_commands.json"
}

# Builds the HeroesRecomp target; configures first if needed.
sr_build() {
  local type="$1"; shift
  local bdir; bdir="$(sr_type_dir "$type")"
  if [ ! -f "$bdir/build.ninja" ]; then
    sr_configure "$type" || return 1
  fi
  cmake --build "$bdir" --target HeroesRecomp "$@"
}

# Runs the built binary.
sr_run() {
  local type="$1"; shift

  # Binary: out/build/nix-<type>/HeroesRecomp/HeroesRecomp.
  local bdir; bdir="$(sr_type_dir "$type")"
  local exe="$bdir/HeroesRecomp/HeroesRecomp"
  [ -x "$exe" ] || { echo "heroesrecomp: $exe not built; run build-$(echo "$type" | tr '[:upper:]' '[:lower:]')" >&2; return 1; }

  exec "$exe" "$@"
}

sr_clean() { rm -rf "$SR_ROOT"/out/build/nix-*; }

sr_help() {
  cat <<'EOS'
HeroesRecomp dev shell.
<type> = debug | relwithdebinfo | release

  Build:
    build-<type>            configure (if needed) + build HeroesRecomp
    configure-<type>        CMake configure only
    sr-clean                delete out/build/nix-*

  Run:
    run-<type>              run built exe with Nix runtime env

  Meta:
    sr-help                 this text
EOS
}

# ------------------------------------------------------------------------------
# Command scripts (field per command; generated into out/nix-cache/bin)
# ------------------------------------------------------------------------------
# On my machine, bash functions are invisible to zsh after `nix develop` swaps
# shells, so each command written out as a script that can be invoked.

declare -A SR_COMMANDS=(
  # Build
  [configure-debug]="sr_configure Debug"
  [configure-relwithdebinfo]="sr_configure RelWithDebInfo"
  [configure-release]="sr_configure Release"
  [build-debug]="sr_build Debug"
  [build-relwithdebinfo]="sr_build RelWithDebInfo"
  [build-release]="sr_build Release"
  # Run
  [run-debug]="sr_run Debug"
  [run-relwithdebinfo]="sr_run RelWithDebInfo"
  [run-release]="sr_run Release"
  # Meta
  [sr-clean]="sr_clean"
  [sr-help]="sr_help"
)

if [ -z "${SR_WRAPPER:-}" ]; then
  SR_BIN="$SR_ROOT/out/nix-cache/bin"
  mkdir -p "$SR_BIN"
  for sr_cmd in "${!SR_COMMANDS[@]}"; do
    # Write to a temp file in the same directory, then rename: a concurrently
    # running shell can never exec a half-written script.
    sr_tmp="$(mktemp "$SR_BIN/$sr_cmd.XXXXXX")"
    printf '#!/usr/bin/env bash\nSR_WRAPPER=1 SR_ROOT="%s"\n. "$SR_ROOT/nix/sr-env.sh"\n%s "$@"\n' \
      "$SR_ROOT" "${SR_COMMANDS[$sr_cmd]}" > "$sr_tmp"
    chmod +x "$sr_tmp"
    mv -f "$sr_tmp" "$SR_BIN/$sr_cmd"
  done
  unset sr_cmd sr_tmp
  # Prepend the script dir to PATH once (no duplicates on re-entry).
  case ":$PATH:" in
    *":$SR_BIN:"*) ;;
    *) export PATH="$SR_BIN:$PATH" ;;
  esac
fi

# ------------------------------------------------------------------------------
# Shell setup (interactive loads only)
# ------------------------------------------------------------------------------

if [ -z "${SR_WRAPPER:-}" ]; then
  # vcpkg is stubbed in Nix; deps come from the dev shell instead. The stub
  # exists because CMakeLists.txt hard-includes $VCPKG_ROOT/.../vcpkg.cmake.
  SR_VCPKG_STUB="$SR_ROOT/out/nix-cache/vcpkg-stub/scripts/buildsystems"
  mkdir -p "$SR_VCPKG_STUB"
  sr_stub="$SR_VCPKG_STUB/vcpkg.cmake"
  if [ ! -f "$sr_stub" ]; then
    printf '# Nix dev-shell stub replacing the vcpkg toolchain.\n' > "$sr_stub"
  fi
  export VCPKG_ROOT="${sr_stub%/*/*/*}"

  # Submodules: checked out automatically (flakes do not fetch them).
  if git -C "$SR_ROOT" submodule status --recursive 2>/dev/null | grep -q '^-'; then
    echo "heroesrecomp: initializing submodules..."
    git -C "$SR_ROOT" submodule update --init --recursive || {
      echo "ERROR: submodule initialization failed." >&2
    }
  fi

  [ -f "$SR_ROOT/HeroesRecompLib/private/default.xbe" ] || {
    echo "WARNING: HeroesRecompLib/private/default.xbe missing - build will fail." >&2
    echo "         Copy it from your own game copy (docs/BUILDING.md)." >&2
  }

  sr_help
fi
