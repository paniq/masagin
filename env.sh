#!/bin/sh
# Shared toolchain setup, sourced by every build script.
# Override anything from the environment: SLANGC, SDL3_CFLAGS, SDL3_LIBS, EXTRA.

MSG_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
SRC="$MSG_ROOT/src"
OBJ="$MSG_ROOT/obj"

# Slang (Slang source -> SPIR-V).  A copy vendored by ./setup_deps.sh wins over a
# system installation; without either, the *.spv committed in shaders/ are used.
if [ -z "$SLANGC" ]; then
  if [ -x "$MSG_ROOT/deps/slang/bin/slangc" ]; then
    SLANGC="$MSG_ROOT/deps/slang/bin/slangc"
  else
    SLANGC=$(command -v slangc 2>/dev/null || true)
  fi
fi

# SDL3 (window, input, audio).  The distro package is picked up through pkg-config;
# ./setup_deps.sh builds deps/sdl3-install where the distro has no SDL3 development
# package, and that prefix is then also searched at run time.
if [ -z "$SDL3_CFLAGS" ] || [ -z "$SDL3_LIBS" ]; then
  if [ -d "$MSG_ROOT/deps/sdl3-install" ]; then
    SDL3_CFLAGS="-I$MSG_ROOT/deps/sdl3-install/include"
    SDL3_LIBS="-L$MSG_ROOT/deps/sdl3-install/lib -L$MSG_ROOT/deps/sdl3-install/lib64 -lSDL3"
    SDL3_LIBS="$SDL3_LIBS -Wl,-rpath,$MSG_ROOT/deps/sdl3-install/lib -Wl,-rpath,$MSG_ROOT/deps/sdl3-install/lib64"
  elif pkg-config --exists sdl3 2>/dev/null; then
    SDL3_CFLAGS=$(pkg-config --cflags sdl3)
    SDL3_LIBS=$(pkg-config --libs sdl3)
  else
    SDL3_CFLAGS=""
    SDL3_LIBS="-lSDL3"
  fi
fi

# The Altona and msglib sources are C++98; vkcore.cpp and sdl_shim.cpp are C++17.
# sCONFIG_RENDER_BLANK selects the renderer slot that graphics_vk.cpp fills.
CXXMSG="-std=gnu++98 -fpermissive -fshort-wchar -fno-exceptions -fno-rtti -msse2 -O1 -g0 -w"
CXXMSG="$CXXMSG -DsCONFIG_RENDER_BLANK -DsCONFIG_BUILD_RELEASE"
CXXMSG="$CXXMSG -I$SRC/altona/main -I$SRC/altona/main/base -I$SRC/libs -I$SRC/libs/msglib -I$SRC"
CXX17="-std=c++17 -O2 -w"
