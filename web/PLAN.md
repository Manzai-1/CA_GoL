Build a web UI for an existing C/SDL2 Game of Life compiled to WASM via Emscripten. Do NOT modify casdl.c or config.h.

CREATE:
- web/shell.html
- web/styles.css
- web/controls.js
Update Makefile to use custom shell and copy CSS/JS to build/.

EXPORTED C FUNCTIONS (call via Module.ccall after Module.onRuntimeInitialized):
- js_pan(dy, dx) void
- js_zoom(delta) void
- js_set_speed(ms) void
- js_get_min_speed() int
- js_get_max_speed() int
- js_toggle_pause() void
- js_get_paused() int (1=paused)

LAYOUT: Bottom control bar, three zones via flexbox:
- Left: vertical zoom stack (+, "ZOOM" label, −)
- Center: large circular play/pause button (~64px), swaps play/pause icon
- Right: speed slider with live "Speed: XXms" label, tiny min/max endpoint labels

AESTHETIC: Monochrome simulation tool. No color (sim canvas uses magenta/blue/red — UI must not compete).
- Bg #0a0a0a, controls bar #141414, top border #222
- Text #e8e8e8 primary, #666 secondary
- Buttons: transparent bg, border #2a2a2a, hover border+icon #fff
- Play button filled: bg #e8e8e8, icon #000
- Slider: track #2a2a2a, thumb #e8e8e8, fully custom-styled (hide browser chrome)
- border-radius: 2px (sharp), except 50% on play button
- Transitions: 100ms linear, color/border only. No gradients, shadows, bounces.

TYPOGRAPHY:
- Labels: system sans, 11px, weight 500, uppercase, letter-spacing 0.08em
- Numbers: JetBrains Mono via Google Fonts, 12px

ICONS: Lucide via CDN (https://unpkg.com/lucide@latest). Use play/pause/plus/minus. Call lucide.createIcons() after any data-lucide change.

MAKEFILE: Add --shell-file web/shell.html to emcc flags. Add cp web/styles.css web/controls.js build/ after build.

BEHAVIOR:
- On Module.onRuntimeInitialized: query speed min/max, set slider range + endpoint labels, init icons, wire events.
- Slider 'input' event: js_set_speed + update label.
- Play click: js_toggle_pause, then query js_get_paused, swap icon.
- Zoom +/−: js_zoom(1) / js_zoom(-1).

ACCESSIBILITY: aria-labels on buttons and slider.

OUT OF SCOPE: generation counter, step button, reset, themes, mobile-specific controls.

Success: make clean && make serve → canvas + polished monochrome control bar, all controls working, SDL mouse interactions still work.