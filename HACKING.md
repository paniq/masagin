# masagin -- hacking notes

Notes for anyone building or changing this: the build in pieces, the content and
shader pipeline, the player switches, the compile-time switches, the renderer
tests, and where the sources are. `make` is the front door, and the header
comment of the `Makefile` lists its targets; watching the show is in README.md.

## What you need

- Linux with a Vulkan 1.3 driver (`vulkaninfo --summary` should list your GPU; the
  software driver `llvmpipe` is enough for the headless tests)
- SDL3, a C++ compiler, `pkg-config`
- Python 3 with numpy, Pillow and PyOpenGL, plus `rsvg-convert` — only for the
  content pipeline
- Slang (`slangc`) — only to rebuild the shaders; the compiled SPIR-V is committed

`make` runs `./setup_deps.sh` before it compiles: it names what is missing and
offers to install it (apt, plus a Slang download into `deps/`), and says nothing
when nothing is missing. `./setup_deps.sh --check` only lists what is missing and
installs nothing. SDL3 gets built into `deps/sdl3-install` on distributions that
have no SDL3 development package.

## Building

    make

`make` compiles the sources into `obj/flags-<flags>/`, links `./masagin`, rebuilds
the shaders when a `slangc` is available (otherwise the `.spv` committed in
`shaders/` are kept) and builds the show. A full build takes a few seconds; after
a change only the objects of that source and of everything including the header
it touched are recompiled. The objects live in a folder named after the compile
flags, so changing the flags can never mix in a stale object. The parts have
targets of their own: `make masagin`, `make shaders`, `make content`, `make deps`,
`make run` (build and play in one step), and `make clean` / `make distclean`.

`env.sh` is sourced by make and by `compile_shaders.sh`, and picks up the
toolchains: a Slang copy under `deps/slang/`, otherwise `slangc` from `$PATH`; SDL3
through `pkg-config`, otherwise the prefix `deps/sdl3-install` that `setup_deps.sh`
builds on distributions without an SDL3 development package. Override either with
`SLANGC`, `SDL3_CFLAGS`, `SDL3_LIBS`.

## Building the show

    make content [SHOW=NAME] [CONTENTFLAGS=-j 4]

`make` builds the show as part of its default goal; this is that step on its own.
`tools/build_content.py` converts `content/<show>/` into a runnable image in
`build/<show>/`:

| source    | output     | tool                 |
|-----------|------------|----------------------|
| `*.svg`   | `*.bsm`    | `tools/svg2bsm.py`   |
| `*.msg`   | `*.msg.o`  | `tools/msgc.py`      |
| `*.ogg`   | copied     |                      |

The folder layout is mirrored, the build is incremental, and the SVG conversions
run in parallel (`CONTENTFLAGS=-j 4`, one converter per shape; each takes some
memory). After the build, every shape referenced by the scripts is checked for a
`.bsm`, because a missing one would only be noticed while the show runs.
`tools/bsmview.py` rasterises a `.bsm` again for a look at a single shape.

## Running

    ./masagin [options] [script.msg.o]        # run this from the project root

`masagin` moves into `build/masagin/` before it opens anything, so the script
argument (default `demo.msg.o`) and everything the scripts reference there
(shapes, audio, `loader.msg.o`) are paths inside the built show. The shaders are
found in `shaders/` next to the program, so nothing needs to point at the
sources. `-show NAME` selects `build/NAME`, `-dir DIR` moves into another folder
instead (`-dir .` for a show copied out on its own), and a relative `-dump DIR`
writes into the show folder.

`make run MSGFLAGS="-window -mb 0"` builds and plays in one step, and
`make run SHOW=NAME` plays another show than the default one.

Keys in a running show: `Esc` always quits; `Tab` switches on the debug view, and
only with it on do `Backspace` (reload the script) and the other scene keys act.
The list is in README.md.

| switch | effect |
|--------|--------|
| (nothing) | fullscreen at the desktop resolution |
| `-window` | 640x400 window; `-w N -h N` sets the window size instead |
| `-f` | force fullscreen |
| `-novsync` | turn vsync off |
| `-mb N` | motion blur: 0 off, 1 16 samples, 2 16 samples interleaved (default), 3 8, 4 8 interleaved |
| `-nobloom` | no bloom |
| `-bloomradius P` | bloom blur radius, percent (default 100) |
| `-bloomstrength P` | bloom intensity, percent (default 100; flat colours stay flat) |
| `-framedrop` | allow dropping frames |
| `-loop` | accepted, but nothing acts on it yet |
| `-shaders DIR` | SPIR-V directory; `MSG_SHADER_DIR` is honoured as well |
| `-headless` | no window; runs on any Vulkan driver, including software (`MSG_HEADLESS=1` also works) |
| `-frames N` | quit after N frames |
| `-dump DIR [-dumpstart S] [-dumpevery E]` | write a PNG of the offscreen frame every E frames, from frame S on |

Environment: `MSG_VK_DEVICE=n` picks the GPU, `MSG_VK_DEBUG=1` prints a command
trace, `MSG_VK_VALIDATE=1` enables the validation layer, `SDL_AUDIODRIVER=dummy`
runs without a sound device.

## Compile-time switches

Passed through `EXTRA` when compiling; they change how the renderer places and
interprets pixel data, which is what the test programs below look at. `EXTRA`
reaches every source, the two C++17 ones included -- `MSG_VK_FLIP_Y_VIEWPORT` is
defaulted inside `vkcore.cpp` -- and the objects built with them are kept in a
folder of their own, so an old object set is never mixed into a new build:

    EXTRA="-DMSG_VK_FLIP_Y_VIEWPORT=0" make test

| define | meaning |
|--------|---------|
| `MSG_VK_FLIP_Y_VIEWPORT` | the viewport carries a negative height so the picture is top-down. Default 1; 0 draws it the other way round |
| `MSG_VK_SPREAD_OFFSET` | how far the glare blur's taps are shifted, in texels of the destination. Default `1.0`; `0` places the taps raw and the blur comes out up-left of its source |
| `MSG_VK_BLIT_TEXEL_OFFSET` | sample the blits half a texel off-centre. Off by default, since Vulkan samples texel centres exactly |
| `MSG_VK_MATRIX_TRANSPOSE` | transpose the matrix upload. Not needed |
| `MSG_VK_CULL_INVERT` | swap which face is considered front-facing. Not needed |

## Tests

    make test        # builds the three programs below, runs them headless and
                     # measures the pictures; dumps go to /tmp/masagin-test
    make vktest      # one program on its own: ./vktest, ./vkbloom, ./corehello

`vktest` draws quads whose expected position, colour, winding, blending and depth
behaviour are written down next to each call, so a wrong transform, a missing Y
flip or an inverted cull shows up as a visible difference. `vkbloom` runs the
bloom sequence on a white square and `test/measure.py` reports where the glare
lands and how wide it is (a centred blur gives a shift around zero). `corehello`
pushes one triangle straight through the renderer core without Altona or content
and has to be run from the repository root, where `./shaders` is found.

## Where things are

    src/masagin/          the player: script selection, the loader, the show loop
    src/libs/msglib/      the runtime the show runs on: compiler-side structures,
                          entity tree, shapes, materials, camera, audio
    src/altona/           the engine layer the player is built on
      main/base/          types, math, files, threads, windowing, renderer
        vkcore.hpp/cpp    Vulkan 1.3 core: dynamic rendering, pipelines, textures,
                          PNG dumps. Plain C++17, no Altona headers
        graphics_vk.cpp   maps Altona's sMaterial/sGeometry/sTexture2D and render
                          targets onto vkcore
        platform_linux.cpp, sdl_shim.*   window, input, audio and files
      main/util/          materials and helpers shared with the engine
    shaders/              the Slang sources (msg, ipp, simple) and their SPIR-V
    content/masagin/      the show: scripts, SVG shapes, audio
    build/masagin/        the built show (not versioned); masagin works in it
    tools/                the content pipeline and the shape/script inspectors
    test/                 the renderer test programs
    Makefile              build front door; env.sh finds the toolchains for it
    docs/scripting_manual.pdf   the `.msg` language, as written by the show's author

Scripts and shapes are written against the same material set the shaders
implement; the constant blocks are declared three times and have to agree: once
in the Slang sources, once in the C++ (`shaders.hpp`, `ippmtrl.hpp`) and once in
Altona shader-compiler syntax (`shaders.asc`, `ippmtrl.asc`), which is kept as
documentation of the layout and is not part of this build.

## Notes

- `deps/`, `obj/`, `build/` and the built programs are not versioned; the
  compiled SPIR-V in `shaders/` is, so a build only needs a compiler and SDL3.
- `make clean` removes the objects and the programs, `make distclean` also the
  built show and the toolchains vendored under `deps/`.
- `masagin --help` prints the same switch list as above.
- The sources carry their original copyright headers; the engine layer is Altona.
