#!/bin/bash
# Make sure the build has what it needs (Debian/Ubuntu). `make deps` runs this
# before anything is compiled: it installs only what is actually missing and
# stays quiet when nothing is, so it is cheap on every build and repairs a
# package that went away since the last one.
#
#   --yes    install without asking (for runs without a terminal)
#   --check  name what is missing, install nothing, exit 1 if anything is
#
# Nothing here runs sudo unless something has to be installed, and a Slang
# download or an SDL3 build from source are only done after they were asked for.
set -e
cd "$(dirname "$0")"
. ./env.sh

mode=
for a in "$@"; do
  case "$a" in
    --yes)   mode=yes;;
    --check) mode=check;;
    *) echo "usage: ./setup_deps.sh [--yes] [--check]"; exit 1;;
  esac
done

# What the build needs. The content pipeline runs on Python image tools and
# rsvg-convert, so those are required for the show; a Slang compiler, the
# validator and a second Vulkan driver are not needed to build or to run, the
# compiled SPIR-V is committed and the system driver is normally there already.
req=()
opt=()
command -v g++        >/dev/null || req+=("build-essential")
command -v pkg-config >/dev/null || req+=("pkg-config")
[ -f /usr/include/vulkan/vulkan.h ] || req+=("libvulkan-dev")
pkg-config --exists sdl3 2>/dev/null || [ -d deps/sdl3-install ] || req+=("libsdl3-dev")
command -v rsvg-convert >/dev/null || req+=("librsvg2-bin")
python3 -c 'import numpy, PIL, OpenGL' >/dev/null 2>&1 || req+=("python3-numpy python3-pil python3-opengl")
command -v spirv-val >/dev/null || opt+=("spirv-tools")
command -v vulkaninfo >/dev/null || opt+=("vulkan-tools mesa-vulkan-drivers")
slang=0; [ -n "$SLANGC" ] || slang=1

if [ ${#req[@]} = 0 ] && [ $slang = 0 ] && [ ${#opt[@]} = 0 ]; then
  exit 0                                   # everything in place, say nothing
fi
[ ${#req[@]} = 0 ] || printf 'missing (needed to build): %s\n' "${req[*]}"
[ ${#opt[@]} = 0 ] || printf 'missing (only to rebuild shaders / for diagnostics): %s\n' "${opt[*]}"
[ $slang = 0 ] || echo 'missing (optional): slangc -- 40 MB download into deps/slang'

if [ "$mode" = check ]; then
  if [ ${#req[@]} = 0 ]; then exit 0; else exit 1; fi
fi

# Without a terminal there is nobody to ask: a missing required package then
# stops the build with a message, missing optional pieces are left alone.
if [ "$mode" != yes ]; then
  if [ ! -t 0 ]; then
    if [ ${#req[@]} = 0 ]; then
      echo "no terminal to ask in: leaving the optional parts alone (or run with --yes)"
      exit 0
    fi
    echo "no terminal to ask in: run ./setup_deps.sh by hand, or with --yes"
    exit 1
  fi
  printf 'install the missing parts (apt with sudo, Slang into deps/)? [Y/n] '
  read -r answer
  case "$answer" in
    [nN]*) echo "left as it is"; [ ${#req[@]} = 0 ] || exit 1;;
  esac
fi

sudo apt-get update
sudo apt-get install -y "${req[@]}" ${opt[@]+"${opt[@]}"}

# Slang (Slang source -> SPIR-V). Not packaged; unpacked into deps/, which is
# not versioned. env.sh picks it up ahead of a system installation.
if [ ! -x deps/slang/bin/slangc ]; then
  SLANG_VERSION=2026.22
  mkdir -p deps/slang
  curl -L -o deps/slang.tgz \
    "https://github.com/shader-slang/slang/releases/download/v$SLANG_VERSION/slang-$SLANG_VERSION-linux-x86_64.tar.gz"
  tar xzf deps/slang.tgz -C deps/slang
fi
echo "slangc: $(cd deps && pwd)/slang/bin/slangc"

# Distros without an SDL3 development package: build one into deps/sdl3-install,
# which env.sh then uses for compiling, linking and running.
if ! pkg-config --exists sdl3 2>/dev/null; then
  sudo apt-get install -y cmake ninja-build \
      libx11-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev \
      libxfixes-dev libxss-dev libxtst-dev libxrender-dev libxinerama-dev libxkbcommon-dev \
      libwayland-dev wayland-protocols libdrm-dev libgbm-dev libasound2-dev libpulse-dev \
      libdecor-0-dev libegl-dev
  mkdir -p deps
  curl -L -o deps/sdl3.tgz https://codeload.github.com/libsdl-org/SDL/tar.gz/refs/tags/release-3.4.20
  tar xzf deps/sdl3.tgz -C deps
  cmake -S deps/SDL-release-3.4.20 -B deps/SDL-release-3.4.20/build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF \
        -DCMAKE_INSTALL_PREFIX="$PWD/deps/sdl3-install"
  cmake --build deps/SDL-release-3.4.20/build
  cmake --install deps/SDL-release-3.4.20/build
fi

./setup_deps.sh --check            # report what is left, if anything
