Module.onRuntimeInitialized = function () {
  const min = Module.ccall('js_get_min_speed', 'number', [], []);
  const max = Module.ccall('js_get_max_speed', 'number', [], []);
  const slider = document.getElementById('speed-slider');
  slider.min = min;
  slider.max = max;
  slider.value = 1000;
  document.getElementById('speed-min').textContent = min + 'ms';
  document.getElementById('speed-max').textContent = max + 'ms';
  document.getElementById('speed-label').textContent = `Speed: ${slider.value}ms`;

  lucide.createIcons();

  const genCounter = document.getElementById('gen-counter');
  setInterval(() => {
    const count = Module.ccall('js_get_generations', 'number', [], []);
    genCounter.textContent = `GEN ${count.toString().padStart(6, '0')}`;
  }, 100);

  slider.addEventListener('input', () => {
    Module.ccall('js_set_speed', null, ['number'], [parseInt(slider.value)]);
    document.getElementById('speed-label').textContent = `Speed: ${slider.value}ms`;
  });

  document.getElementById('play-pause').addEventListener('click', () => {
    Module.ccall('js_toggle_pause', null, [], []);
    const paused = Module.ccall('js_get_paused', 'number', [], []);
    const btn = document.getElementById('play-pause');
    btn.querySelector('[data-lucide]').setAttribute('data-lucide', paused ? 'play' : 'pause');
    btn.setAttribute('aria-label', paused ? 'Play' : 'Pause');
    lucide.createIcons();
  });

  document.getElementById('zoom-in').addEventListener('click', () => {
    Module.ccall('js_zoom', null, ['number'], [1]);
  });

  document.getElementById('zoom-out').addEventListener('click', () => {
    Module.ccall('js_zoom', null, ['number'], [-1]);
  });
};
