# Game of Life

Conway's Game of Life in C with SDL2, compiled to WebAssembly via Emscripten.

[![Build and Deploy](https://github.com/Manzai-1/CA_GoL/actions/workflows/deploy.yml/badge.svg)](https://github.com/Manzai-1/CA_GoL/actions/workflows/deploy.yml)

**[Follow this link for live demo](https://manzai-1.github.io/CA_GoL/)**

![Game of Life running](media/gameoflife.gif)

800 × 800 grid. Four cell states (birth, stasis, death, empty) rendered via direct pixel-buffer streaming to a GPU texture.

## Controls

| Action | Control |
|--------|---------|
| Pan    | Mouse drag |
| Zoom   | Scroll wheel, or `+` / `-` buttons |
| Pause  | Spacebar, or play/pause button |
| Speed  | Slider (10 - 2000 ms per generation) |

UI buttons call exported C functions via `Module.ccall`.

## Design notes

**Fixed timestep, no catch-up.** Rendering runs every animation frame, generations advance on a user-controlled ms interval via an accumulator. The accumulator absorbs the mismatch between browser refresh rate and simulation speed, and resets on each generation rather than decrementing. Skipping generations to catch up would negate the purpose of this simulation. 

**Double-buffered grid.** Cells compute into a secondary buffer, then `memcpy` back. Updating in place corrupts neighbor reads.

**Direct pixel-buffer rendering.** Per-cell `SDL_RenderDrawPoint` doesn't scale. The grid is written as raw ARGB into a locked texture in one pass and drawn as a single textured quad.

**Single `AppState` struct.** Grid pointers, SDL handles, view & sim configs, and timing live in one heap-allocated struct passed to `emscripten_set_main_loop_arg`. A file-scope pointer exists only for JS-exported functions, which can't receive runtime arguments.

## Build

Requires the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html).

```bash
make             # compile to build/
make serve       # build and serve on localhost:8000
make clean
```

Pushes to `main` trigger `.github/workflows/deploy.yml`, which builds on an Ubuntu runner and publishes `build/` to GitHub Pages.

## Structure

```
casdl.c              simulation
config.h             compile-time constants
Makefile
web/
  shell.html         Emscripten HTML template + UI
  styles.css
  controls.js        JavaScript and C wiring
.github/workflows/
  deploy.yml
```

## Limitations

- Grid size fixed at compile time.
- No pattern loader.
- Integer division in zoom centering causes +-1 cell drift per step.
- Simulation complexity is O(W*H), every cell checks its 3x3 neighborhood in each generation. Caching techniques such as Hashlife could be used to optimize, but that is out of scope for this project.
- Grid edges dont wrap, cells at boundary have fewer than 8 neighbors due to this choice.
- Single threaded, the update loop could easily be optimized with parallelization. This is out of scope for this project considering that WASM threads require advanced configuration of SharedArrayBuffer.

## License

MIT