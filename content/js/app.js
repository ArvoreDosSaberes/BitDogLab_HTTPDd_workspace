/**
 * BitDogLab - JavaScript AJAX Controller
 * Handles communication with the Pico W HTTP server
 */

// LED Matrix state (25 LEDs in a 5x5 grid)
let matrixState = new Array(25).fill(null);
let selectedColor = '#ff0000';
let animationInterval = null;

// OLED lines buffer
let oledLines = ['', '', '', '', '', '', '', ''];

// Polling interval (ms)
const POLL_INTERVAL = 200;

// ============================================
// Initialization
// ============================================

document.addEventListener('DOMContentLoaded', () => {
    initLedMatrix();
    initOledInput();
    startPolling();
    updateRGB();
});

// ============================================
// LED Matrix Functions
// ============================================

function initLedMatrix() {
    const matrix = document.getElementById('led-matrix');
    matrix.innerHTML = '';
    
    for (let i = 0; i < 25; i++) {
        const led = document.createElement('div');
        led.className = 'led-cell';
        led.dataset.index = i;
        led.onclick = () => toggleLed(i);
        matrix.appendChild(led);
    }
}

function toggleLed(index) {
    const led = document.querySelector(`[data-index="${index}"]`);
    if (matrixState[index]) {
        matrixState[index] = null;
        led.style.backgroundColor = '#1a1a1a';
        led.classList.remove('active');
    } else {
        matrixState[index] = selectedColor;
        led.style.backgroundColor = selectedColor;
        led.classList.add('active');
    }
}

function clearMatrix(keepAnimation = false) {
    if (!keepAnimation) stopAnimation();
    matrixState.fill(null);
    document.querySelectorAll('.led-cell').forEach(led => {
        led.style.backgroundColor = '#1a1a1a';
        led.classList.remove('active');
    });
}

function fillMatrix() {
    stopAnimation();
    selectedColor = document.getElementById('led-color').value;
    matrixState.fill(selectedColor);
    document.querySelectorAll('.led-cell').forEach(led => {
        led.style.backgroundColor = selectedColor;
        led.classList.add('active');
    });
}

// Convert XY to serpentine index
function xyToIndex(x, y) {
    if (y % 2 === 0) {
        return y * 5 + x;
    } else {
        return y * 5 + (4 - x);
    }
}

// Flip index vertically (hardware has Y inverted)
function flipIndex(i) {
    const row = Math.floor(i / 5);
    const col = i % 5;
    return (4 - row) * 5 + col;
}

// Preset patterns (defined for visual grid, will be flipped when sent)
const presets = {
    // Símbolos
    heart: [1, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 22],
    smile: [6, 8, 11, 13, 15, 19, 21, 22, 23],
    x: [0, 4, 6, 8, 12, 16, 18, 20, 24],
    check: [4, 8, 12, 16, 20],
    // Setas
    arrow_up: [2, 6, 7, 8, 12, 17, 22],
    arrow_down: [2, 7, 12, 16, 17, 18, 22],
    arrow_left: [2, 6, 10, 11, 12, 13, 14, 16, 22],
    arrow_right: [2, 8, 10, 11, 12, 13, 14, 18, 22],
    // Formas
    square: [0, 1, 2, 3, 4, 5, 9, 10, 14, 15, 19, 20, 21, 22, 23, 24],
    diamond: [2, 6, 8, 10, 14, 16, 18, 22],
    plus: [2, 7, 10, 11, 12, 13, 14, 17, 22]
};

function setPreset(name) {
    stopAnimation();
    clearMatrix();
    selectedColor = document.getElementById('led-color').value;
    const pattern = presets[name];
    if (pattern) {
        pattern.forEach(index => {
            matrixState[index] = selectedColor;
            const led = document.querySelector(`[data-index="${index}"]`);
            led.style.backgroundColor = selectedColor;
            led.classList.add('active');
        });
        sendMatrix(); // Auto-send after preset
    }
}

function sendMatrix() {
    // Convert matrix state to sendable format with vertical flip
    const flippedData = new Array(25);
    for (let i = 0; i < 25; i++) {
        flippedData[flipIndex(i)] = matrixState[i];
    }
    
    const data = flippedData.map(color => {
        if (!color) return '000000';
        return color.replace('#', '');
    }).join(',');
    
    fetch('/matrix.cgi', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `data=${encodeURIComponent(data)}`
    }).then(response => {
        if (response.ok) {
            console.log('Matrix updated');
        }
    }).catch(err => console.error('Matrix send error:', err));
}

// Stop any running animation
function stopAnimation() {
    if (animationInterval) {
        clearInterval(animationInterval);
        animationInterval = null;
    }
}

// Helper to update LED visually
function setLedVisual(idx, color) {
    matrixState[idx] = color;
    const led = document.querySelector(`[data-index="${idx}"]`);
    if (led) {
        led.style.backgroundColor = color;
        led.classList.add('active');
    }
}

// Wave effect from top to bottom
function waveTopBottom() {
    stopAnimation();
    selectedColor = document.getElementById('led-color').value;
    let row = 0;
    animationInterval = setInterval(() => {
        clearMatrix(true);
        for (let col = 0; col < 5; col++) {
            setLedVisual(row * 5 + col, selectedColor);
        }
        sendMatrix();
        row = (row + 1) % 5;
    }, 200);
}

// Wave effect from bottom to top
function waveBottomTop() {
    stopAnimation();
    selectedColor = document.getElementById('led-color').value;
    let row = 4;
    animationInterval = setInterval(() => {
        clearMatrix(true);
        for (let col = 0; col < 5; col++) {
            setLedVisual(row * 5 + col, selectedColor);
        }
        sendMatrix();
        row = (row - 1 + 5) % 5;
    }, 200);
}

// Wave effect from left to right
function waveLeftRight() {
    stopAnimation();
    selectedColor = document.getElementById('led-color').value;
    let col = 0;
    animationInterval = setInterval(() => {
        clearMatrix(true);
        for (let row = 0; row < 5; row++) {
            setLedVisual(row * 5 + col, selectedColor);
        }
        sendMatrix();
        col = (col + 1) % 5;
    }, 200);
}

// Wave effect from right to left
function waveRightLeft() {
    stopAnimation();
    selectedColor = document.getElementById('led-color').value;
    let col = 4;
    animationInterval = setInterval(() => {
        clearMatrix(true);
        for (let row = 0; row < 5; row++) {
            setLedVisual(row * 5 + col, selectedColor);
        }
        sendMatrix();
        col = (col - 1 + 5) % 5;
    }, 200);
}

// Expanding wave from center
function waveExpand() {
    stopAnimation();
    selectedColor = document.getElementById('led-color').value;
    const rings = [
        [12],
        [6, 7, 8, 11, 13, 16, 17, 18],
        [0, 1, 2, 3, 4, 5, 9, 10, 14, 15, 19, 20, 21, 22, 23, 24]
    ];
    let ring = 0;
    animationInterval = setInterval(() => {
        clearMatrix(true);
        rings[ring].forEach(idx => setLedVisual(idx, selectedColor));
        sendMatrix();
        ring = (ring + 1) % rings.length;
    }, 300);
}

// Update selected color
document.addEventListener('DOMContentLoaded', () => {
    const colorPicker = document.getElementById('led-color');
    if (colorPicker) {
        colorPicker.addEventListener('change', (e) => {
            selectedColor = e.target.value;
        });
    }
});

// ============================================
// RGB Effects - Advanced Color Effects
// ============================================

// Helper: Convert HSV to RGB
function hsvToRgb(h, s, v) {
    let r, g, b;
    const i = Math.floor(h * 6);
    const f = h * 6 - i;
    const p = v * (1 - s);
    const q = v * (1 - f * s);
    const t = v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return {
        r: Math.round(r * 255),
        g: Math.round(g * 255),
        b: Math.round(b * 255)
    };
}

// Helper: RGB to Hex
function rgbToHexColor(r, g, b) {
    return '#' + [r, g, b].map(x => x.toString(16).padStart(2, '0')).join('');
}

// Effect: Rainbow Wave - Arco-iris percorrendo a matriz
function effectRainbow() {
    stopAnimation();
    let offset = 0;
    animationInterval = setInterval(() => {
        for (let i = 0; i < 25; i++) {
            const row = Math.floor(i / 5);
            const col = i % 5;
            const hue = ((col + row + offset) % 10) / 10;
            const rgb = hsvToRgb(hue, 1, 1);
            setLedVisual(i, rgbToHexColor(rgb.r, rgb.g, rgb.b));
        }
        sendMatrix();
        offset = (offset + 1) % 10;
    }, 150);
}

// Effect: Rainbow Spiral - Espiral arco-iris do centro
function effectRainbowSpiral() {
    stopAnimation();
    const spiral = [12, 7, 11, 17, 13, 8, 6, 16, 18, 2, 1, 5, 10, 15, 20, 21, 22, 23, 19, 14, 9, 4, 3, 0, 24];
    let offset = 0;
    animationInterval = setInterval(() => {
        for (let i = 0; i < 25; i++) {
            const hue = ((spiral.indexOf(i) + offset) % 25) / 25;
            const rgb = hsvToRgb(hue, 1, 1);
            setLedVisual(i, rgbToHexColor(rgb.r, rgb.g, rgb.b));
        }
        sendMatrix();
        offset = (offset + 1) % 25;
    }, 100);
}

// Effect: Color Chase - Cores perseguindo
function effectColorChase() {
    stopAnimation();
    const colors = ['#ff0000', '#00ff00', '#0000ff', '#ffff00', '#ff00ff', '#00ffff'];
    let pos = 0;
    let colorIdx = 0;
    animationInterval = setInterval(() => {
        clearMatrix(true);
        setLedVisual(pos, colors[colorIdx]);
        setLedVisual((pos + 1) % 25, colors[(colorIdx + 1) % colors.length]);
        setLedVisual((pos + 2) % 25, colors[(colorIdx + 2) % colors.length]);
        sendMatrix();
        pos = (pos + 1) % 25;
        if (pos === 0) colorIdx = (colorIdx + 1) % colors.length;
    }, 80);
}

// Effect: Breathing - Respiracao com cor selecionada
function effectBreathing() {
    stopAnimation();
    selectedColor = document.getElementById('led-color').value;
    const baseRgb = hexToRgb(selectedColor);
    let brightness = 0;
    let increasing = true;
    animationInterval = setInterval(() => {
        const factor = brightness / 100;
        const r = Math.round(baseRgb.r * factor);
        const g = Math.round(baseRgb.g * factor);
        const b = Math.round(baseRgb.b * factor);
        const color = rgbToHexColor(r, g, b);
        for (let i = 0; i < 25; i++) {
            setLedVisual(i, color);
        }
        sendMatrix();
        if (increasing) {
            brightness += 5;
            if (brightness >= 100) increasing = false;
        } else {
            brightness -= 5;
            if (brightness <= 0) increasing = true;
        }
    }, 50);
}

// Effect: Fire - Simulacao de fogo
function effectFire() {
    stopAnimation();
    animationInterval = setInterval(() => {
        for (let i = 0; i < 25; i++) {
            const row = Math.floor(i / 5);
            // Mais vermelho no topo, mais amarelo na base
            const intensity = Math.random() * 0.5 + 0.5;
            const r = Math.round(255 * intensity);
            const g = Math.round((50 + (4 - row) * 40) * intensity * Math.random());
            const b = 0;
            setLedVisual(i, rgbToHexColor(r, g, b));
        }
        sendMatrix();
    }, 100);
}

// Effect: Sparkle - Brilhos aleatorios
function effectSparkle() {
    stopAnimation();
    animationInterval = setInterval(() => {
        clearMatrix(true);
        // Acende 3-5 LEDs aleatorios com cores aleatorias
        const numSparkles = 3 + Math.floor(Math.random() * 3);
        for (let s = 0; s < numSparkles; s++) {
            const idx = Math.floor(Math.random() * 25);
            const hue = Math.random();
            const rgb = hsvToRgb(hue, 1, 1);
            setLedVisual(idx, rgbToHexColor(rgb.r, rgb.g, rgb.b));
        }
        sendMatrix();
    }, 100);
}

// Effect: Color Wheel - Roda de cores rotativa
function effectColorWheel() {
    stopAnimation();
    let rotation = 0;
    animationInterval = setInterval(() => {
        for (let i = 0; i < 25; i++) {
            const row = Math.floor(i / 5);
            const col = i % 5;
            // Distancia do centro
            const dx = col - 2;
            const dy = row - 2;
            const angle = Math.atan2(dy, dx) / (2 * Math.PI) + 0.5;
            const hue = (angle + rotation) % 1;
            const rgb = hsvToRgb(hue, 1, 1);
            setLedVisual(i, rgbToHexColor(rgb.r, rgb.g, rgb.b));
        }
        sendMatrix();
        rotation = (rotation + 0.05) % 1;
    }, 100);
}

// Effect: Gradient - Gradiente diagonal
function effectGradient() {
    stopAnimation();
    let offset = 0;
    const color1 = hexToRgb(document.getElementById('led-color').value);
    // Cor complementar
    const color2 = { r: 255 - color1.r, g: 255 - color1.g, b: 255 - color1.b };
    animationInterval = setInterval(() => {
        for (let i = 0; i < 25; i++) {
            const row = Math.floor(i / 5);
            const col = i % 5;
            const t = ((row + col + offset) % 8) / 8;
            const r = Math.round(color1.r + (color2.r - color1.r) * t);
            const g = Math.round(color1.g + (color2.g - color1.g) * t);
            const b = Math.round(color1.b + (color2.b - color1.b) * t);
            setLedVisual(i, rgbToHexColor(r, g, b));
        }
        sendMatrix();
        offset = (offset + 1) % 8;
    }, 150);
}

// Effect: Pulse - Pulso colorido do centro
function effectPulse() {
    stopAnimation();
    let step = 0;
    const colors = ['#ff0000', '#ff7700', '#ffff00', '#00ff00', '#0077ff', '#8800ff'];
    animationInterval = setInterval(() => {
        clearMatrix(true);
        const ring = step % 3;
        const colorIdx = Math.floor(step / 3) % colors.length;
        const rings = [
            [12],
            [6, 7, 8, 11, 13, 16, 17, 18],
            [0, 1, 2, 3, 4, 5, 9, 10, 14, 15, 19, 20, 21, 22, 23, 24]
        ];
        rings[ring].forEach(idx => setLedVisual(idx, colors[colorIdx]));
        sendMatrix();
        step = (step + 1) % (3 * colors.length);
    }, 150);
}

// Effect: Random Colors - Cores aleatorias em toda matriz
function effectRandomColors() {
    stopAnimation();
    animationInterval = setInterval(() => {
        for (let i = 0; i < 25; i++) {
            const hue = Math.random();
            const rgb = hsvToRgb(hue, 1, 1);
            setLedVisual(i, rgbToHexColor(rgb.r, rgb.g, rgb.b));
        }
        sendMatrix();
    }, 200);
}

// Effect: Matrix Rain - Chuva estilo Matrix
function effectMatrixRain() {
    stopAnimation();
    const drops = [0, 0, 0, 0, 0];
    animationInterval = setInterval(() => {
        clearMatrix(true);
        for (let col = 0; col < 5; col++) {
            // Desenha a gota com fade
            for (let trail = 0; trail < 3; trail++) {
                const row = (drops[col] - trail + 5) % 5;
                const idx = row * 5 + col;
                const brightness = 1 - trail * 0.3;
                const g = Math.round(255 * brightness);
                setLedVisual(idx, rgbToHexColor(0, g, 0));
            }
            // Move a gota
            if (Math.random() > 0.3) {
                drops[col] = (drops[col] + 1) % 5;
            }
        }
        sendMatrix();
    }, 120);
}

// Effect: Ocean Waves - Ondas do oceano
function effectOceanWaves() {
    stopAnimation();
    let phase = 0;
    animationInterval = setInterval(() => {
        for (let i = 0; i < 25; i++) {
            const row = Math.floor(i / 5);
            const col = i % 5;
            const wave = Math.sin((col + phase) * 0.8) * 0.5 + 0.5;
            const depth = (4 - row) / 4;
            const r = 0;
            const g = Math.round(100 * wave * depth);
            const b = Math.round(200 + 55 * wave);
            setLedVisual(i, rgbToHexColor(r, g, b));
        }
        sendMatrix();
        phase += 0.5;
    }, 100);
}

// Effect: Lava Lamp - Lampada de lava
function effectLavaLamp() {
    stopAnimation();
    const blobs = [
        { x: 1, y: 2, hue: 0 },
        { x: 3, y: 1, hue: 0.3 },
        { x: 2, y: 3, hue: 0.6 }
    ];
    animationInterval = setInterval(() => {
        clearMatrix(true);
        blobs.forEach(blob => {
            // Move blob
            blob.x += (Math.random() - 0.5) * 0.8;
            blob.y += (Math.random() - 0.5) * 0.8;
            blob.x = Math.max(0, Math.min(4, blob.x));
            blob.y = Math.max(0, Math.min(4, blob.y));
            blob.hue = (blob.hue + 0.02) % 1;
            // Draw blob
            for (let i = 0; i < 25; i++) {
                const row = Math.floor(i / 5);
                const col = i % 5;
                const dist = Math.sqrt(Math.pow(col - blob.x, 2) + Math.pow(row - blob.y, 2));
                if (dist < 1.8) {
                    const brightness = 1 - dist / 1.8;
                    const rgb = hsvToRgb(blob.hue, 1, brightness);
                    setLedVisual(i, rgbToHexColor(rgb.r, rgb.g, rgb.b));
                }
            }
        });
        sendMatrix();
    }, 100);
}

// Effect: Police Lights - Luzes de policia
function effectPolice() {
    stopAnimation();
    let state = 0;
    animationInterval = setInterval(() => {
        clearMatrix(true);
        if (state < 2) {
            // Azul na esquerda
            for (let row = 0; row < 5; row++) {
                for (let col = 0; col < 2; col++) {
                    setLedVisual(row * 5 + col, '#0000ff');
                }
            }
        } else {
            // Vermelho na direita
            for (let row = 0; row < 5; row++) {
                for (let col = 3; col < 5; col++) {
                    setLedVisual(row * 5 + col, '#ff0000');
                }
            }
        }
        sendMatrix();
        state = (state + 1) % 4;
    }, 150);
}

// Effect: Disco - Disco ball
function effectDisco() {
    stopAnimation();
    animationInterval = setInterval(() => {
        clearMatrix(true);
        // Acende LEDs aleatorios com cores vivas
        const colors = ['#ff0000', '#00ff00', '#0000ff', '#ffff00', '#ff00ff', '#00ffff', '#ffffff'];
        for (let i = 0; i < 25; i++) {
            if (Math.random() > 0.6) {
                setLedVisual(i, colors[Math.floor(Math.random() * colors.length)]);
            }
        }
        sendMatrix();
    }, 100);
}

// ============================================
// OLED Functions
// ============================================

function initOledInput() {
    const input = document.getElementById('oled-text');
    if (input) {
        input.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                sendOledText();
            }
        });
    }
}

function sendOledText() {
    const input = document.getElementById('oled-text');
    const text = input.value.trim();
    
    if (text) {
        // Shift lines up
        for (let i = 0; i < 7; i++) {
            oledLines[i] = oledLines[i + 1];
        }
        oledLines[7] = text;
        
        // Update preview
        updateOledPreview();
        
        // Send to device
        fetch('/oled.cgi', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: `text=${encodeURIComponent(text)}`
        }).then(response => {
            if (response.ok) {
                console.log('OLED text sent');
            }
        }).catch(err => console.error('OLED send error:', err));
        
        input.value = '';
    }
}

function updateOledPreview() {
    for (let i = 0; i < 8; i++) {
        const line = document.getElementById(`oled-line-${i}`);
        if (line) {
            line.textContent = oledLines[i] || '';
        }
    }
}

// ============================================
// Buzzer Functions
// ============================================

function updateBuzzerValues() {
    const freq = document.getElementById('buzzer-freq').value;
    const dur = document.getElementById('buzzer-dur').value;
    
    document.getElementById('buzzer-freq-val').textContent = freq;
    document.getElementById('buzzer-dur-val').textContent = dur;
}

function playBuzzer(freq, dur) {
    // If no parameters, use slider values
    if (freq === undefined) {
        freq = document.getElementById('buzzer-freq').value;
    }
    if (dur === undefined) {
        dur = document.getElementById('buzzer-dur').value;
    }
    
    // Get selected channel
    const channel = document.getElementById('buzzer-channel').value;
    
    fetch('/buzzer.cgi', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `freq=${freq}&dur=${dur}&ch=${channel}`
    }).then(response => {
        if (response.ok) {
            console.log(`Buzzer [${channel}]: ${freq}Hz, ${dur}ms`);
        }
    }).catch(err => console.error('Buzzer send error:', err));
}

// ============================================
// RGB LED Functions
// ============================================

function updateRGB() {
    const r = document.getElementById('r-slider').value;
    const g = document.getElementById('g-slider').value;
    const b = document.getElementById('b-slider').value;
    
    document.getElementById('r-val').textContent = r;
    document.getElementById('g-val').textContent = g;
    document.getElementById('b-val').textContent = b;
    
    const preview = document.getElementById('rgb-preview');
    preview.style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
    preview.style.boxShadow = `0 0 30px rgba(${r}, ${g}, ${b}, 0.5)`;
}

function sendRGB() {
    const r = document.getElementById('r-slider').value;
    const g = document.getElementById('g-slider').value;
    const b = document.getElementById('b-slider').value;
    
    fetch('/rgb.cgi', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `r=${r}&g=${g}&b=${b}`
    }).then(response => {
        if (response.ok) {
            console.log('RGB updated');
        }
    }).catch(err => console.error('RGB send error:', err));
}

// ============================================
// State Polling (AJAX)
// ============================================

function startPolling() {
    pollState();
    setInterval(pollState, POLL_INTERVAL);
}

function pollState() {
    fetch('/state.shtml')
        .then(response => response.text())
        .then(html => {
            // Parse the simple state response
            const parser = new DOMParser();
            const doc = parser.parseFromString(html, 'text/html');
            
            // Update button states
            updateButtonState('a', doc.getElementById('btna')?.textContent);
            updateButtonState('b', doc.getElementById('btnb')?.textContent);
            
            // Update joystick
            const joyX = doc.getElementById('joyx')?.textContent || '2048';
            const joyY = doc.getElementById('joyy')?.textContent || '2048';
            const joyBtn = doc.getElementById('joybtn')?.textContent || '1';
            updateJoystick(parseInt(joyX), parseInt(joyY), joyBtn === '0');
            
            // Update uptime
            const uptime = doc.getElementById('uptime')?.textContent;
            if (uptime) {
                document.getElementById('uptime').textContent = uptime;
            }
            
            // Update temperature gauge
            const temp = doc.getElementById('temp')?.textContent;
            if (temp) {
                updateTemperatureGauge(parseFloat(temp));
            }
        })
        .catch(err => {
            // Silent fail for polling
        });
}

function updateButtonState(btn, state) {
    const indicator = document.getElementById(`btn-${btn}-indicator`);
    const stateEl = document.getElementById(`btn-${btn}-state`);
    const statusEl = document.getElementById(`btn-${btn}-status`);
    
    const isPressed = state === '0' || state === 'Pressionado';
    
    if (indicator) {
        if (isPressed) {
            indicator.classList.add('pressed');
        } else {
            indicator.classList.remove('pressed');
        }
    }
    
    if (stateEl) {
        stateEl.textContent = isPressed ? 'Pressionado' : 'Solto';
    }
    
    if (statusEl) {
        statusEl.textContent = isPressed ? 'Pressionado' : 'Solto';
        statusEl.className = 'status-badge' + (isPressed ? ' pressed' : '');
    }
}

function updateJoystick(x, y, btnPressed) {
    // Update values display
    document.getElementById('joy-x-val').textContent = x;
    document.getElementById('joy-y-val').textContent = y;
    document.getElementById('joy-btn-val').textContent = btnPressed ? 'Pressionado' : 'Solto';
    
    // Update status badges
    document.getElementById('joy-x-status').textContent = x;
    document.getElementById('joy-y-status').textContent = y;
    document.getElementById('joy-btn-status').textContent = btnPressed ? 'Pressionado' : 'Solto';
    
    // Update visual joystick position
    // ADC range is 0-4095, center is ~2048
    const stick = document.getElementById('joystick-stick');
    if (stick) {
        // Normalize to -1 to 1 range
        const normX = (x - 2048) / 2048;
        const normY = (y - 2048) / 2048;
        
        // Map to pixel offset (max 35px from center)
        const offsetX = normX * 35;
        const offsetY = normY * 35;
        
        stick.style.transform = `translate(calc(-50% + ${offsetX}px), calc(-50% + ${offsetY}px))`;
        
        // Change color if button pressed
        if (btnPressed) {
            stick.style.background = 'linear-gradient(145deg, #ef4444, #dc2626)';
        } else {
            stick.style.background = 'linear-gradient(145deg, #60a5fa, #2563eb)';
        }
    }
}

// ============================================
// Utility Functions
// ============================================

function hexToRgb(hex) {
    const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? {
        r: parseInt(result[1], 16),
        g: parseInt(result[2], 16),
        b: parseInt(result[3], 16)
    } : null;
}

function rgbToHex(r, g, b) {
    return '#' + [r, g, b].map(x => {
        const hex = x.toString(16);
        return hex.length === 1 ? '0' + hex : hex;
    }).join('');
}

// ============================================
// Temperature Gauge Functions
// ============================================

function updateTemperatureGauge(temp) {
    // Update displayed value
    const tempValEl = document.getElementById('temp-val');
    if (tempValEl) {
        tempValEl.textContent = temp.toFixed(1);
    }
    
    // Clamp temperature to 0-100 range for gauge
    const clampedTemp = Math.max(0, Math.min(100, temp));
    
    // Update gauge needle rotation (-90° to 90°, where -90° is 0°C and 90° is 100°C)
    const needleAngle = -90 + (clampedTemp / 100) * 180;
    const needleEl = document.getElementById('gauge-needle');
    if (needleEl) {
        needleEl.style.transform = `translateX(-50%) rotate(${needleAngle}deg)`;
    }
    
    // Update gauge fill arc
    const gaugeFill = document.getElementById('gauge-fill');
    if (gaugeFill) {
        // Calculate arc endpoint based on temperature
        // Arc goes from (20, 100) to (180, 100) with center at (100, 100), radius 80
        const angleRad = (Math.PI * clampedTemp) / 100;
        const endX = 100 - 80 * Math.cos(angleRad);
        const endY = 100 - 80 * Math.sin(angleRad);
        
        // Determine if we need the large arc flag (for temps > 50°C)
        const largeArc = clampedTemp > 50 ? 1 : 0;
        
        // Create the arc path
        const pathD = `M 20 100 A 80 80 0 ${largeArc} 1 ${endX.toFixed(1)} ${endY.toFixed(1)}`;
        gaugeFill.setAttribute('d', pathD);
        
        // Change color based on temperature
        let color;
        if (temp < 30) {
            color = '#22c55e'; // Green - cool
        } else if (temp < 50) {
            color = '#eab308'; // Yellow - warm
        } else if (temp < 70) {
            color = '#f97316'; // Orange - hot
        } else {
            color = '#ef4444'; // Red - very hot
        }
        gaugeFill.style.stroke = color;
    }
}
