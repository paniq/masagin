# Masagin by Neuro & Farbrausch

Masagin is a demo show: sixteen music scenes laid out in the `.msg` scripting
language, shapes drawn as SVG and converted to triangle meshes, one continuous
flow through all of them. The player is a small C++ application that renders with
Vulkan 1.3 and plays through SDL3.

## Watching it

    make run

One command, typed in the project folder: it installs what is missing (asking
first, and using sudo for that), compiles the player, converts the show into
`build/masagin/`, and plays it fullscreen at the desktop resolution. `Esc` quits.

The one thing your machine has to provide is a working Vulkan 1.3 driver --
`vulkaninfo --summary` (from `vulkan-tools`) should name your GPU. A display is
not needed to show that it works:

    ./masagin -headless -frames 30 -dump /tmp/masagin

renders thirty frames into PNGs. Once the player is built, `./masagin --help`
lists the switches: `-window` for a window instead of fullscreen, `-w N -h N` for
a size, `-mb 0 -nobloom` if the picture wants too much of the machine.

### Keys

`Esc` always quits. `Tab` switches on the scene's own controls, and only with
them on: `Space` pause, `Left`/`Right` step through time, `Backspace` reload the
script, `b` bloom, `m` motion blur, `l` loop, `x` frame dropping, `f` antialiasing.

## Working on it

HACKING.md is the rest: the dependency list, the build in pieces, the content and
shader pipeline, every player switch and environment variable, the compile-time
switches, the renderer test programs and where the sources are.
`docs/scripting_manual.pdf` describes the `.msg` language, as written by the
show's author.

The sources carry their original copyright headers; the engine layer is Altona.
