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
