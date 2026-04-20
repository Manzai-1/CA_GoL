Add a generation counter overlay to the top-left corner of the canvas.

C export available: Module.ccall('js_get_generations', 'number', [], []) returns an int.

1. In shell.html, add inside body before #controls:
   <div id="gen-counter" class="mono">GEN 000000</div>

2. In styles.css, add:
   - Position absolute, top-left of the viewport with ~16px spacing from top and left
   - Font: JetBrains Mono, thick weight (700+), white (#fff), ~14-16px
   - pointer-events: none so it doesn't block canvas interactions
   - No background, no border
   - z-index above canvas

3. In controls.js, inside Module.onRuntimeInitialized, add a setInterval (100ms) that reads js_get_generations and updates #gen-counter textContent as `GEN ${count.toString().padStart(6, '0')}`.

4. Ensure the JetBrains Mono font link in shell.html includes weight 700 — update the href to include multiple weights like &wght@400;700.