/**
 * ============================================================
 * CENTINELA v2.0 — APP WEB DE CONTROL DEL BASTÓN INTELIGENTE
 * ============================================================
 * Arquitectura: Vanilla JS, estado centralizado en appState
 * Módulos: bluetooth, gps, ui, speech, commands (inline)
 * ============================================================
 */

'use strict';

/* ============================================================
   ESTADO CENTRALIZADO DE LA APP
   ============================================================ */
const appState = {
    // Conexión BT
    bt: {
        connected: false,
        connecting: false,
        deviceName: null,
        streaming: false,
    },
    // Sensores
    sensors: {
        distFrontal: null,    // cm
        distLateral: null,    // cm
        radarAngle: 90,       // grados
        radarSector: 'CENTRO',
    },
    // Batería
    battery: {
        percent: null,
        charging: false,
    },
    // GPS
    gps: {
        lat: null,
        lon: null,
        fix: false,
        sats: 0,
        accuracy: null,
        timestamp: null,
        history: [],         // últimas 5 posiciones
    },
    // Estado del bastón (evento actual)
    event: 'LIBRE',          // LIBRE | OBSTACULO_IZQ | OBSTACULO_DER | POZO | CAIDA | EMERGENCIA
    direction: 'CENTRO',

    // UI / preferencias
    ui: {
        activeTab: 'home',
        fontSize: 'normal',
        voiceAlertsOn: true,
        radarOn: true,
        threshold: 60,       // cm
        navActive: false,
        voiceCmdActive: false,
        volume: 70,
    },
    // Emergencia
    sos: {
        active: false,
        countdown: null,
        timer: null,
    },
    // Contacto de emergencia
    contact: {
        name: 'María García',
        phone: '+5491123456789',
    },
    // Simulación BT
    sim: {
        intervalId: null,
        tickCount: 0,
    },
    // Speech
    speech: {
        synth: window.speechSynthesis || null,
        recognition: null,
        speaking: false,
        lastAnnouncement: '',
        lastAnnouncementTime: 0,
    },
};

/* ============================================================
   MÓDULO: UTILIDADES
   ============================================================ */
const utils = {
    /** Vibrar (háptica simulada) con patrón */
    vibrate(pattern) {
        if (navigator.vibrate) {
            navigator.vibrate(pattern);
        }
    },

    /** Formatear coordenada decimal para lectura */
    formatCoord(val, decimals = 6) {
        if (val === null || val === undefined) return '---';
        return val.toFixed(decimals);
    },

    /** Hora actual HH:MM:SS */
    timeNow() {
        return new Date().toLocaleTimeString('es-AR', { hour12: false });
    },

    /** Guardar preferencias en localStorage */
    savePrefs() {
        try {
            localStorage.setItem('centinela_ui', JSON.stringify(appState.ui));
            localStorage.setItem('centinela_contact', JSON.stringify(appState.contact));
        } catch (e) { /* ignorar */ }
    },

    /** Cargar preferencias de localStorage */
    loadPrefs() {
        try {
            const ui = localStorage.getItem('centinela_ui');
            const contact = localStorage.getItem('centinela_contact');
            if (ui) Object.assign(appState.ui, JSON.parse(ui));
            if (contact) Object.assign(appState.contact, JSON.parse(contact));
        } catch (e) { /* ignorar */ }
    },

    /** Debounce */
    debounce(fn, delay) {
        let t;
        return (...args) => {
            clearTimeout(t);
            t = setTimeout(() => fn(...args), delay);
        };
    },
};

/* ============================================================
   MÓDULO: SPEECH (síntesis y reconocimiento de voz)
   ============================================================ */
const speech = {
    /** Hablar en español — sin repetir anuncios idénticos en < 3s */
    say(text, priority = false) {
        if (!appState.speech.synth || !appState.ui.voiceAlertsOn) return;

        const now = Date.now();
        if (!priority &&
            text === appState.speech.lastAnnouncement &&
            now - appState.speech.lastAnnouncementTime < 3000) return;

        appState.speech.lastAnnouncement = text;
        appState.speech.lastAnnouncementTime = now;

        // Cancelar lo que esté hablando solo si es prioritario
        if (priority) {
            appState.speech.synth.cancel();
        }

        const utt = new SpeechSynthesisUtterance(text);
        utt.lang = 'es-AR';
        utt.rate = 1.05;
        utt.pitch = 1.0;
        utt.volume = appState.ui.volume / 100;

        // Seleccionar voz española si está disponible
        const voices = appState.speech.synth.getVoices();
        const esVoice = voices.find(v =>
            v.lang.startsWith('es') && v.localService
        ) || voices.find(v => v.lang.startsWith('es'));
        if (esVoice) utt.voice = esVoice;

        appState.speech.synth.speak(utt);
    },

    /** Anunciar por aria-live region (para lectores de pantalla) */
    announce(text, urgent = false) {
        const el = document.getElementById(urgent ? 'sr-alerts' : 'sr-status');
        if (el) {
            el.textContent = '';
            requestAnimationFrame(() => {
                el.textContent = text;
            });
        }
    },

    /** Iniciar reconocimiento de voz */
    startRecognition() {
        const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
        if (!SpeechRecognition) {
            speech.say('Lo siento, tu navegador no soporta comandos de voz.');
            ui.showToast('Comandos de voz no disponibles en este navegador', 'warning');
            return;
        }

        if (appState.speech.recognition) {
            appState.speech.recognition.stop();
        }

        const rec = new SpeechRecognition();
        rec.lang = 'es-AR';
        rec.continuous = false;
        rec.interimResults = false;
        rec.maxAlternatives = 3;

        rec.onstart = () => {
            appState.ui.voiceCmdActive = true;
            ui.updateVoiceBtn();
            speech.say('Escuchando. Di un comando.');
            ui.showToast('Escuchando...', 'info');
        };

        rec.onresult = (e) => {
            const results = Array.from(e.results[0]).map(r => r.transcript.toLowerCase().trim());
            voiceCommands.process(results);
        };

        rec.onerror = (e) => {
            console.warn('Speech error:', e.error);
            if (e.error === 'no-speech') {
                speech.say('No escuché nada. Inténtalo de nuevo.');
            }
        };

        rec.onend = () => {
            appState.ui.voiceCmdActive = false;
            appState.speech.recognition = null;
            ui.updateVoiceBtn();
        };

        appState.speech.recognition = rec;
        rec.start();
    },

    stopRecognition() {
        if (appState.speech.recognition) {
            appState.speech.recognition.stop();
            appState.speech.recognition = null;
        }
        appState.ui.voiceCmdActive = false;
        ui.updateVoiceBtn();
    },
};

/* ============================================================
   MÓDULO: COMANDOS DE VOZ
   ============================================================ */
const voiceCommands = {
    process(texts) {
        // Tomar el mejor resultado
        const text = texts.join(' ');

        if (text.includes('emergencia') || text.includes('auxilio') || text.includes('sos')) {
            sos.activate();
            return;
        }
        if (text.includes('estado') || text.includes('qué pasa') || text.includes('como estoy')) {
            const msg = `Estado actual: ${appState.event.replace(/_/g, ' ')}. ` +
                        `Batería: ${appState.battery.percent ?? '?'}%. ` +
                        `GPS: ${appState.gps.fix ? 'fijado' : 'buscando satélites'}.`;
            speech.say(msg, true);
            speech.announce(msg);
            return;
        }
        if (text.includes('batería') || text.includes('carga') || text.includes('energia')) {
            const msg = appState.battery.percent !== null
                ? `Nivel de batería: ${appState.battery.percent} por ciento`
                : 'Batería sin datos. Conecta el bastón.';
            speech.say(msg, true);
            speech.announce(msg);
            return;
        }
        if (text.includes('ubicación') || text.includes('dónde estoy') || text.includes('posición')) {
            const lat = utils.formatCoord(appState.gps.lat, 5);
            const lon = utils.formatCoord(appState.gps.lon, 5);
            const msg = appState.gps.fix
                ? `Tu ubicación GPS: latitud ${lat}, longitud ${lon}`
                : 'GPS sin señal. Estoy buscando satélites.';
            speech.say(msg, true);
            speech.announce(msg);
            return;
        }
        if (text.includes('conectar') || text.includes('conexión') || text.includes('bluetooth')) {
            bluetooth.showOverlay();
            speech.say('Buscando el bastón por Bluetooth.');
            return;
        }
        if (text.includes('navegar') || text.includes('guía')) {
            nav.toggle();
            return;
        }
        // Sin coincidencia
        speech.say(`No reconocí el comando: ${texts[0]}. Di emergencia, estado, batería, ubicación o conectar.`);
    },
};

/* ============================================================
   MÓDULO: UI — Actualizaciones de la interfaz
   ============================================================ */
const ui = {
    /** Cambiar de pestaña */
    switchTab(tabName) {
        const allTabs = document.querySelectorAll('.tab-btn');
        const allPanels = document.querySelectorAll('.tab-panel');

        allTabs.forEach(t => {
            const active = t.dataset.tab === tabName;
            t.classList.toggle('active', active);
            t.setAttribute('aria-selected', String(active));
        });

        allPanels.forEach(p => {
            const panelId = p.id.replace('panel-', '');
            const active = panelId === tabName;
            p.classList.toggle('active', active);
            p.classList.toggle('hidden', !active);
        });

        appState.ui.activeTab = tabName;
    },

    /** Actualizar banner de evento principal */
    updateEventBanner() {
        const banner = document.getElementById('event-banner');
        const iconEl = document.getElementById('event-icon');
        const textEl = document.getElementById('event-text');
        const subEl  = document.getElementById('event-sub');

        const eventMap = {
            LIBRE:          { icon: '🟢', text: 'LIBRE',               sub: 'Sin obstáculos detectados',       cls: 'state-libre'         },
            OBSTACULO_IZQ:  { icon: '⬅️', text: 'OBSTÁCULO IZQUIERDA', sub: 'Objeto detectado a tu izquierda', cls: 'state-obstaculo-izq' },
            OBSTACULO_DER:  { icon: '➡️', text: 'OBSTÁCULO DERECHA',   sub: 'Objeto detectado a tu derecha',   cls: 'state-obstaculo-der' },
            OBSTACULO_FRONT:{ icon: '⬆️', text: 'OBSTÁCULO FRONTAL',   sub: 'Objeto al frente — detente',      cls: 'state-obstaculo-izq' },
            POZO:           { icon: '⚠️', text: 'PELIGRO: POZO',       sub: 'Desnivel o pozo detectado',       cls: 'state-pozo'          },
            CAIDA:          { icon: '🔴', text: '¡CAÍDA DETECTADA!',   sub: 'Llamando al contacto…',           cls: 'state-caida'         },
            EMERGENCIA:     { icon: '🆘', text: 'EMERGENCIA SOS',       sub: 'Enviando alerta con GPS',         cls: 'state-emergencia'    },
        };

        const ev = eventMap[appState.event] || eventMap['LIBRE'];

        // Quitar todas las clases de estado
        banner.classList.remove(
            'state-libre','state-obstaculo-izq','state-obstaculo-der',
            'state-pozo','state-caida','state-emergencia'
        );
        banner.classList.add(ev.cls);

        iconEl.textContent = ev.icon;
        textEl.textContent = ev.text;
        subEl.textContent  = ev.sub;
    },

    /** Actualizar chips de estado en el header */
    updateStatusChips() {
        const btChip   = document.getElementById('bt-chip');
        const gpsChip  = document.getElementById('gps-chip');
        const batChip  = document.getElementById('battery-chip');

        // BT
        const btText = document.getElementById('bt-status-text');
        if (appState.bt.connected) {
            btChip.classList.add('connected');
            btChip.classList.remove('error');
            btText.textContent = 'Conectado';
        } else {
            btChip.classList.remove('connected');
            btText.textContent = 'Sin BT';
        }

        // GPS
        const gpsText = document.getElementById('gps-status-text');
        if (appState.gps.fix) {
            gpsChip.classList.add('connected');
            gpsChip.classList.remove('warning');
            gpsText.textContent = 'GPS OK';
        } else {
            gpsChip.classList.remove('connected');
            gpsChip.classList.add('warning');
            gpsText.textContent = 'Sin fix';
        }

        // Batería
        const batPct = document.getElementById('battery-pct');
        if (appState.battery.percent !== null) {
            batPct.textContent = `${appState.battery.percent}%`;
            const p = appState.battery.percent;
            batChip.classList.toggle('connected', p > 30);
            batChip.classList.toggle('warning', p > 15 && p <= 30);
            batChip.classList.toggle('error', p <= 15);
        } else {
            batPct.textContent = 'BAT';
        }
    },

    /** Actualizar sensor cards */
    updateSensorCards() {
        // Distancia frontal
        const dist = appState.sensors.distFrontal;
        const distEl = document.getElementById('dist-frontal');
        const barEl  = document.getElementById('dist-bar');

        if (dist !== null) {
            distEl.textContent = `${Math.round(dist)} cm`;
            const pct = Math.min(100, (dist / 200) * 100);
            barEl.style.width = `${pct}%`;
            barEl.classList.remove('warn', 'danger');
            if (dist < 30) barEl.classList.add('danger');
            else if (dist < 60) barEl.classList.add('warn');
        } else {
            distEl.textContent = '--- cm';
            barEl.style.width = '0%';
        }

        // Batería grande
        const batValEl  = document.getElementById('battery-value');
        const batFillEl = document.getElementById('battery-fill');
        const pct = appState.battery.percent;

        if (pct !== null) {
            batValEl.textContent = `${pct}%`;
            batFillEl.style.width = `${pct}%`;
            batValEl.classList.remove('low','critical');
            batFillEl.classList.remove('low','critical');
            if (pct <= 15) {
                batValEl.classList.add('critical');
                batFillEl.classList.add('critical');
            } else if (pct <= 30) {
                batValEl.classList.add('low');
                batFillEl.classList.add('low');
            }
        } else {
            batValEl.textContent = '---%';
            batFillEl.style.width = '0%';
        }

        // GPS fix display
        const gpsFixEl = document.getElementById('gps-fix-display');
        const gpsSatsEl = document.getElementById('gps-sats');
        gpsFixEl.textContent = appState.gps.fix ? '✅ Señal fijada' : '🔍 Buscando...';
        gpsSatsEl.textContent = appState.gps.sats;

        // Radar
        const radarEl = document.getElementById('radar-status');
        if (!appState.ui.radarOn) {
            radarEl.textContent = 'Desactivado';
        } else {
            radarEl.textContent = appState.bt.connected
                ? `${appState.sensors.radarAngle}° — ${appState.sensors.radarSector}`
                : 'Sin conexión BT';
        }
    },

    /** Actualizar panel de telemetría BT */
    updateBtPanel() {
        const dot = document.getElementById('bt-dot');
        const lbl = document.getElementById('bt-stream-label');

        if (appState.bt.connected && appState.bt.streaming) {
            dot.classList.add('active');
            lbl.textContent = '● LIVE';
            lbl.classList.add('active');
        } else {
            dot.classList.remove('active');
            lbl.textContent = appState.bt.connected ? 'Conectado' : 'Sin conexión';
            lbl.classList.remove('active');
        }

        const ev = document.getElementById('bt-evento');
        const dir = document.getElementById('bt-direccion');
        const dist = document.getElementById('bt-dist');
        const bat = document.getElementById('bt-bat');
        const gps = document.getElementById('bt-gps');
        const aur = document.getElementById('bt-auricular');

        if (appState.bt.connected) {
            ev.textContent  = appState.event;
            dir.textContent = appState.direction;
            dist.textContent = appState.sensors.distFrontal !== null
                ? `${Math.round(appState.sensors.distFrontal)} cm` : '--- cm';
            bat.textContent  = appState.battery.percent !== null
                ? `${appState.battery.percent}%` : '---%';
            gps.textContent  = appState.gps.fix
                ? `${utils.formatCoord(appState.gps.lat,4)}, ${utils.formatCoord(appState.gps.lon,4)}`
                : 'Buscando...';
            aur.textContent  = '---';
        } else {
            [ev,dir,dist,bat,gps,aur].forEach(el => { el.textContent = '---'; });
        }
    },

    /** Botón BT en controls */
    updateBtBtn() {
        const btn = document.getElementById('bt-connect-btn');
        const lbl = document.getElementById('bt-btn-label');
        if (appState.bt.connected) {
            btn.classList.add('connected');
            lbl.textContent = 'Desconectar';
            btn.setAttribute('aria-label', 'Desconectar el bastón Bluetooth');
            btn.setAttribute('aria-pressed', 'true');
        } else if (appState.bt.connecting) {
            btn.classList.remove('connected');
            lbl.textContent = 'Buscando...';
            btn.setAttribute('aria-pressed', 'false');
        } else {
            btn.classList.remove('connected');
            lbl.textContent = 'Conectar';
            btn.setAttribute('aria-label', 'Conectar el bastón por Bluetooth');
            btn.setAttribute('aria-pressed', 'false');
        }
    },

    /** Botón de comandos de voz */
    updateVoiceBtn() {
        const btn = document.getElementById('voice-cmd-btn');
        const lbl = document.getElementById('voice-btn-label');
        if (appState.ui.voiceCmdActive) {
            btn.classList.add('active');
            btn.setAttribute('aria-pressed', 'true');
            lbl.textContent = 'Escuchando';
        } else {
            btn.classList.remove('active');
            btn.setAttribute('aria-pressed', 'false');
            lbl.textContent = 'Voz';
        }
    },

    /** Botón de navegación */
    updateNavBtn() {
        const btn  = document.getElementById('nav-toggle-btn');
        const lbl  = document.getElementById('nav-btn-label');
        const bigBtn  = document.getElementById('nav-start-big-btn');
        const bigLbl  = document.getElementById('nav-start-label');

        if (appState.ui.navActive) {
            btn.classList.add('active');
            btn.setAttribute('aria-pressed', 'true');
            lbl.textContent = 'Detener Guía';
            if (bigBtn) {
                bigBtn.setAttribute('aria-pressed', 'true');
                bigLbl.textContent = 'Detener Navegación';
            }
        } else {
            btn.classList.remove('active');
            btn.setAttribute('aria-pressed', 'false');
            lbl.textContent = 'Iniciar Guía';
            if (bigBtn) {
                bigBtn.setAttribute('aria-pressed', 'false');
                bigLbl.textContent = 'Iniciar Navegación';
            }
        }
    },

    /** Panel de mapa GPS */
    updateMapPanel() {
        document.getElementById('map-lat').textContent =
            appState.gps.lat !== null ? utils.formatCoord(appState.gps.lat) : 'Obteniendo...';
        document.getElementById('map-lon').textContent =
            appState.gps.lon !== null ? utils.formatCoord(appState.gps.lon) : 'Obteniendo...';
        document.getElementById('map-accuracy').textContent =
            appState.gps.accuracy !== null ? `±${Math.round(appState.gps.accuracy)} m` : '±--- m';
        document.getElementById('map-time').textContent =
            appState.gps.timestamp || '---';

        // Link a Google Maps
        const mapsLink = document.getElementById('maps-link');
        if (appState.gps.lat !== null && appState.gps.lon !== null) {
            mapsLink.href = `https://www.google.com/maps?q=${appState.gps.lat},${appState.gps.lon}`;
        }
    },

    /** Historial de GPS */
    updateGpsHistory() {
        const list = document.getElementById('gps-history-list');
        if (!appState.gps.history.length) {
            list.innerHTML = '<li class="history-empty">Sin historial aún</li>';
            return;
        }
        list.innerHTML = appState.gps.history.map((pos, i) => `
            <li>
                <a class="history-item"
                   href="https://www.google.com/maps?q=${pos.lat},${pos.lon}"
                   target="_blank" rel="noopener"
                   aria-label="Ubicación ${i + 1}: lat ${utils.formatCoord(pos.lat, 4)}, lon ${utils.formatCoord(pos.lon, 4)}, a las ${pos.time}">
                    <div class="history-num" aria-hidden="true">${i + 1}</div>
                    <div class="history-coords">
                        ${utils.formatCoord(pos.lat, 5)}, ${utils.formatCoord(pos.lon, 5)}
                    </div>
                    <div class="history-time">${pos.time}</div>
                </a>
            </li>`).join('');
    },

    /** Mostrar toast/notificación temporal */
    showToast(message, type = 'info') {
        // Remover toast previo si existe
        const prev = document.querySelector('.toast');
        if (prev) prev.remove();

        const toast = document.createElement('div');
        toast.className = `toast toast-${type}`;
        toast.textContent = message;
        toast.setAttribute('role', 'status');
        toast.setAttribute('aria-live', 'polite');

        const colors = { info: '#C5A059', warning: '#DFBE7D', error: '#D32F2F', success: '#2E7D32' };
        const color = colors[type] || colors.info;

        toast.style.cssText = `
            position: fixed;
            bottom: 90px;
            left: 50%;
            transform: translateX(-50%);
            background: #2D1119;
            border: 1px solid ${color};
            color: ${color};
            padding: 12px 20px;
            border-radius: 12px;
            font-size: 0.9rem;
            font-weight: 700;
            z-index: 1000;
            max-width: 90vw;
            text-align: center;
            animation: toast-in 0.3s ease;
            box-shadow: 0 4px 20px rgba(0,0,0,0.5);
        `;

        // Agregar keyframe si no existe
        if (!document.getElementById('toast-style')) {
            const s = document.createElement('style');
            s.id = 'toast-style';
            s.textContent = `
                @keyframes toast-in { from { opacity:0; transform:translateX(-50%) translateY(10px);} to { opacity:1; transform:translateX(-50%) translateY(0);} }
                @keyframes toast-out { from { opacity:1;} to { opacity:0; transform:translateX(-50%) translateY(10px);} }
            `;
            document.head.appendChild(s);
        }

        document.body.appendChild(toast);
        setTimeout(() => {
            toast.style.animation = 'toast-out 0.3s ease forwards';
            setTimeout(() => toast.remove(), 350);
        }, 3000);
    },

    /** Actualizar tamaño de fuente */
    applyFontSize(size) {
        document.documentElement.setAttribute('data-font-size', size);
        appState.ui.fontSize = size;
        document.querySelectorAll('.font-size-btn').forEach(b => {
            const active = b.dataset.size === size;
            b.classList.toggle('active', active);
            b.setAttribute('aria-pressed', String(active));
        });
        utils.savePrefs();
    },

    /** Render completo de toda la UI */
    renderAll() {
        ui.updateEventBanner();
        ui.updateStatusChips();
        ui.updateSensorCards();
        ui.updateBtPanel();
        ui.updateBtBtn();
        ui.updateNavBtn();
        ui.updateMapPanel();
        ui.updateGpsHistory();
        ui.updateVoiceBtn();
        ui.applyFontSize(appState.ui.fontSize);

        // Volumen
        const volSlider = document.getElementById('volume-slider');
        const volDisplay = document.getElementById('volume-display');
        if (volSlider) volSlider.value = appState.ui.volume;
        if (volDisplay) volDisplay.textContent = `${appState.ui.volume}%`;

        // Umbral
        const thrSlider = document.getElementById('threshold-slider');
        const thrVal = document.getElementById('threshold-val');
        if (thrSlider) thrSlider.value = appState.ui.threshold;
        if (thrVal) thrVal.textContent = `${appState.ui.threshold} cm`;

        // Toggle radar
        const radarToggle = document.getElementById('radar-toggle');
        if (radarToggle) radarToggle.setAttribute('aria-checked', String(appState.ui.radarOn));

        // Toggle alertas de voz
        const voiceToggle = document.getElementById('voice-alerts-toggle');
        if (voiceToggle) voiceToggle.setAttribute('aria-checked', String(appState.ui.voiceAlertsOn));

        // Contacto
        const cName = document.getElementById('contact-name');
        const cPhone = document.getElementById('contact-phone');
        if (cName) cName.value = appState.contact.name;
        if (cPhone) cPhone.value = appState.contact.phone;

        // Call link
        const callBtn = document.getElementById('call-contact-btn');
        if (callBtn) callBtn.href = `tel:${appState.contact.phone}`;
    },
};

/* ============================================================
   MÓDULO: BLUETOOTH — Simulación de protocolo
   ============================================================ */
const bluetooth = {
    showOverlay() {
        const overlay = document.getElementById('bt-overlay');
        overlay.classList.remove('hidden');

        // Simular escaneo y encontrar dispositivos después de 1.5s
        appState.bt.connecting = true;
        ui.updateBtBtn();

        setTimeout(() => {
            const list = document.getElementById('bt-devices-list');
            const scanStatus = document.getElementById('bt-scan-status');

            scanStatus.innerHTML = `
                <svg viewBox="0 0 24 24" width="20" height="20" fill="#2E7D32"><path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z"/></svg>
                <span style="color:#2E7D32">Dispositivo encontrado</span>
            `;

            list.innerHTML = `
                <div class="bt-device-item" role="listitem"
                     id="bt-device-centinela" tabindex="0"
                     aria-label="Dispositivo CENTINELA-ESP32 con señal fuerte. Presiona para conectar">
                    <div>
                        <div class="bt-device-name">CENTINELA-ESP32</div>
                        <div class="bt-device-signal">Señal: ████░ Fuerte</div>
                    </div>
                    <button class="btn btn-primary" style="padding:10px 16px;font-size:0.8rem;"
                            onclick="bluetooth.connect()"
                            aria-label="Conectar con CENTINELA-ESP32">
                        Conectar
                    </button>
                </div>
            `;
        }, 1500);
    },

    hideOverlay() {
        const overlay = document.getElementById('bt-overlay');
        overlay.classList.add('hidden');
    },

    connect() {
        bluetooth.hideOverlay();

        // Simular handshake de 800ms
        setTimeout(() => {
            appState.bt.connected = true;
            appState.bt.connecting = false;
            appState.bt.deviceName = 'CENTINELA-ESP32';
            appState.bt.streaming = true;

            ui.updateBtBtn();
            ui.updateBtPanel();
            ui.updateStatusChips();

            speech.say('Bastón conectado. Iniciando telemetría.');
            speech.announce('Bastón Centinela conectado por Bluetooth.');
            ui.showToast('✅ Bastón conectado', 'success');
            utils.vibrate([100, 50, 100]);

            // Iniciar simulación de datos del bastón
            bluetooth.startSimulation();
        }, 800);
    },

    disconnect() {
        clearInterval(appState.sim.intervalId);
        appState.bt.connected = false;
        appState.bt.streaming = false;
        appState.bt.deviceName = null;
        appState.sensors.distFrontal = null;
        appState.battery.percent = null;

        ui.updateBtBtn();
        ui.updateBtPanel();
        ui.updateStatusChips();
        ui.updateSensorCards();

        speech.say('Bastón desconectado.');
        ui.showToast('Bastón desconectado', 'warning');
    },

    /** Simulación de datos BT con setInterval */
    startSimulation() {
        let tick = 0;

        // Secuencia de eventos simulados para demo realista
        const eventSequence = [
            { t: 0,  event: 'LIBRE',           dir: 'CENTRO', dist: 180, sats: 6 },
            { t: 5,  event: 'LIBRE',           dir: 'CENTRO', dist: 160, sats: 7 },
            { t: 10, event: 'OBSTACULO_FRONT', dir: 'CENTRO', dist: 75,  sats: 7 },
            { t: 15, event: 'OBSTACULO_FRONT', dir: 'CENTRO', dist: 45,  sats: 7 },
            { t: 20, event: 'OBSTACULO_FRONT', dir: 'CENTRO', dist: 25,  sats: 8 },
            { t: 25, event: 'LIBRE',           dir: 'CENTRO', dist: 140, sats: 8 },
            { t: 30, event: 'OBSTACULO_IZQ',   dir: 'IZQ',   dist: 130, sats: 7 },
            { t: 35, event: 'LIBRE',           dir: 'CENTRO', dist: 180, sats: 7 },
            { t: 40, event: 'POZO',            dir: 'CENTRO', dist: 150, sats: 6 },
            { t: 45, event: 'LIBRE',           dir: 'CENTRO', dist: 170, sats: 6 },
            { t: 50, event: 'OBSTACULO_DER',   dir: 'DER',   dist: 120, sats: 7 },
            { t: 55, event: 'LIBRE',           dir: 'CENTRO', dist: 190, sats: 8 },
        ];

        let batPercent = 82;
        let lat = -34.6037 + (Math.random() - 0.5) * 0.01;
        let lon = -58.3816 + (Math.random() - 0.5) * 0.01;
        let radarAngle = 90;
        let radarDir = 1;

        const simId = setInterval(() => {
            if (!appState.bt.connected) {
                clearInterval(simId);
                return;
            }

            tick++;
            appState.sim.tickCount = tick;

            // Evento de la secuencia (loop)
            const seqIndex = tick % (eventSequence[eventSequence.length - 1].t + 5);
            let currentSeq = eventSequence[0];
            for (const step of eventSequence) {
                if (seqIndex >= step.t) currentSeq = step;
                else break;
            }

            const prevEvent = appState.event;
            appState.event = currentSeq.event;
            appState.direction = currentSeq.dir;

            // Distancia con ruido aleatorio pequeño
            const dist = currentSeq.dist + (Math.random() - 0.5) * 8;
            appState.sensors.distFrontal = Math.max(5, dist);

            // Radar
            radarAngle += radarDir * 5;
            if (radarAngle >= 135) radarDir = -1;
            if (radarAngle <= 45) radarDir = 1;
            appState.sensors.radarAngle = Math.round(radarAngle);
            appState.sensors.radarSector =
                radarAngle < 75 ? 'IZQUIERDA' :
                radarAngle > 105 ? 'DERECHA' : 'CENTRO';

            // Batería (decrece lentamente)
            if (tick % 30 === 0 && batPercent > 1) batPercent--;
            appState.battery.percent = batPercent;

            // GPS — moverse ligeramente
            lat += (Math.random() - 0.5) * 0.0001;
            lon += (Math.random() - 0.5) * 0.0001;
            appState.gps.lat = lat;
            appState.gps.lon = lon;
            appState.gps.fix = currentSeq.sats >= 4;
            appState.gps.sats = currentSeq.sats;
            appState.gps.accuracy = Math.round(5 + Math.random() * 10);
            appState.gps.timestamp = utils.timeNow();

            // Guardar historial GPS cada 15 ticks
            if (tick % 15 === 0 && appState.gps.fix) {
                appState.gps.history.unshift({ lat, lon, time: utils.timeNow() });
                if (appState.gps.history.length > 5) {
                    appState.gps.history.pop();
                }
            }

            // ——— Disparar alertas de voz cuando cambia el evento ———
            if (prevEvent !== appState.event) {
                alerts.onEventChange(prevEvent, appState.event);
            }

            // Alerta de batería baja
            if (batPercent === 15) {
                speech.say(`Atención. Batería baja: ${batPercent} por ciento. Busca un punto de carga.`, true);
                speech.announce(`Batería baja: ${batPercent}%`, true);
                utils.vibrate([300, 100, 300, 100, 300]);
            }

            // Render completo de UI
            ui.updateEventBanner();
            ui.updateStatusChips();
            ui.updateSensorCards();
            ui.updateBtPanel();

            // Si estamos en el tab de mapa, actualizar también
            if (appState.ui.activeTab === 'map') {
                ui.updateMapPanel();
                ui.updateGpsHistory();
            }

        }, 800); // Actualiza cada 800ms (simulando telemetría BT a 150ms real)

        appState.sim.intervalId = simId;
    },
};

/* ============================================================
   MÓDULO: ALERTAS — Voz + háptica según evento
   ============================================================ */
const alerts = {
    onEventChange(prev, current) {
        const alertMap = {
            OBSTACULO_IZQ:   { msg: 'Obstáculo a la izquierda.',         vib: [200, 100, 200] },
            OBSTACULO_DER:   { msg: 'Obstáculo a la derecha.',           vib: [200, 100, 200] },
            OBSTACULO_FRONT: { msg: 'Obstáculo al frente. Detente.',     vib: [400, 100, 400] },
            POZO:            { msg: 'Peligro. Pozo detectado. Cuidado.', vib: [600, 150, 600, 150, 600] },
            CAIDA:           { msg: 'Caída detectada. Llamando a tu contacto de emergencia.', vib: [1000] },
            EMERGENCIA:      { msg: 'Emergencia activa. Enviando tu ubicación.', vib: [500, 100, 500, 100, 500] },
            LIBRE:           { msg: 'Camino libre.',                     vib: [50] },
        };

        const alert = alertMap[current];
        if (!alert) return;

        speech.say(alert.msg, current !== 'LIBRE');
        speech.announce(alert.msg, current === 'CAIDA' || current === 'EMERGENCIA');
        utils.vibrate(alert.vib);

        // Si es caída → activar SOS automáticamente
        if (current === 'CAIDA') {
            setTimeout(() => {
                if (appState.event === 'CAIDA' && !appState.sos.active) {
                    sos.activate();
                }
            }, 2000);
        }
    },
};

/* ============================================================
   MÓDULO: GPS — Geolocalización real del navegador
   ============================================================ */
const gps = {
    watchId: null,

    start() {
        if (!navigator.geolocation) {
            ui.showToast('GPS no disponible en este dispositivo', 'warning');
            return;
        }

        gps.watchId = navigator.geolocation.watchPosition(
            (pos) => {
                // Usar coords reales cuando no está conectado el bastón BT
                if (!appState.bt.connected) {
                    appState.gps.lat = pos.coords.latitude;
                    appState.gps.lon = pos.coords.longitude;
                    appState.gps.accuracy = pos.coords.accuracy;
                    appState.gps.fix = true;
                    appState.gps.sats = 0;
                    appState.gps.timestamp = utils.timeNow();

                    // Historial
                    appState.gps.history.unshift({
                        lat: pos.coords.latitude,
                        lon: pos.coords.longitude,
                        time: utils.timeNow(),
                    });
                    if (appState.gps.history.length > 5) appState.gps.history.pop();

                    ui.updateMapPanel();
                    ui.updateGpsHistory();
                    ui.updateStatusChips();
                    ui.updateSensorCards();

                    // Actualizar link SOS
                    const sosMapLink = document.getElementById('sos-maps-link');
                    if (sosMapLink) {
                        sosMapLink.href =
                            `https://www.google.com/maps?q=${pos.coords.latitude},${pos.coords.longitude}`;
                    }
                }
            },
            (err) => {
                console.warn('GPS error:', err.message);
                if (!appState.bt.connected) {
                    appState.gps.fix = false;
                    ui.updateStatusChips();
                    ui.updateSensorCards();
                }
            },
            { enableHighAccuracy: true, timeout: 15000, maximumAge: 5000 }
        );
    },

    stop() {
        if (gps.watchId !== null) {
            navigator.geolocation.clearWatch(gps.watchId);
            gps.watchId = null;
        }
    },

    forceUpdate() {
        if (!navigator.geolocation) return;
        navigator.geolocation.getCurrentPosition(
            (pos) => {
                appState.gps.lat = pos.coords.latitude;
                appState.gps.lon = pos.coords.longitude;
                appState.gps.accuracy = pos.coords.accuracy;
                appState.gps.fix = true;
                appState.gps.timestamp = utils.timeNow();
                appState.gps.history.unshift({
                    lat: pos.coords.latitude, lon: pos.coords.longitude, time: utils.timeNow(),
                });
                if (appState.gps.history.length > 5) appState.gps.history.pop();

                ui.updateMapPanel();
                ui.updateGpsHistory();
                ui.updateStatusChips();
                ui.updateSensorCards();
                speech.say('Ubicación GPS actualizada.');
                ui.showToast('✅ GPS actualizado', 'success');
            },
            (err) => {
                ui.showToast('No se pudo obtener el GPS. Verifica los permisos.', 'error');
            },
            { enableHighAccuracy: true, timeout: 10000, maximumAge: 0 }
        );
    },
};

/* ============================================================
   MÓDULO: SOS — Emergencia
   ============================================================ */
const sos = {
    /** Activar pantalla de emergencia */
    activate() {
        if (appState.sos.active) return;
        appState.sos.active = true;
        appState.event = 'EMERGENCIA';

        // Háptica inmediata fuerte
        utils.vibrate([500, 100, 500, 100, 500, 100, 500]);

        // Voz
        speech.say('¡Emergencia activada! Enviando tu ubicación.', true);
        speech.announce('EMERGENCIA SOS activada. Enviando ubicación GPS.', true);

        // Mostrar overlay SOS
        const overlay = document.getElementById('sos-overlay');
        overlay.classList.remove('hidden');

        // Llenar datos GPS
        const latDisplay = document.getElementById('sos-lat-display');
        const lonDisplay = document.getElementById('sos-lon-display');
        const mapsLink   = document.getElementById('sos-maps-link');
        const callBtn    = document.getElementById('sos-call-btn');

        if (appState.gps.lat !== null) {
            latDisplay.textContent = `Lat: ${utils.formatCoord(appState.gps.lat)}`;
            lonDisplay.textContent = `Lon: ${utils.formatCoord(appState.gps.lon)}`;
            mapsLink.href = `https://www.google.com/maps?q=${appState.gps.lat},${appState.gps.lon}`;
        } else {
            latDisplay.textContent = 'Lat: Obteniendo GPS...';
            lonDisplay.textContent = 'Lon: Obteniendo GPS...';
        }

        callBtn.href = `tel:${appState.contact.phone}`;
        callBtn.setAttribute('aria-label', `Llamar a ${appState.contact.name}: ${appState.contact.phone}`);

        // Countdown de 10s para llamar automáticamente
        let countdown = 10;
        const countdownEl = document.getElementById('sos-countdown');
        if (countdownEl) countdownEl.textContent = countdown;

        appState.sos.timer = setInterval(() => {
            countdown--;
            if (countdownEl) countdownEl.textContent = countdown;
            if (countdown > 0 && countdown <= 3) {
                speech.say(`${countdown}`, true);
            }
            if (countdown <= 0) {
                clearInterval(appState.sos.timer);
                // En un entorno real dispararía la llamada automática
                speech.say(`Llamando a ${appState.contact.name}.`, true);
                window.location.href = `tel:${appState.contact.phone}`;
            }
        }, 1000);

        // Actualizar banner
        ui.updateEventBanner();
        ui.updateStatusChips();
    },

    /** Cancelar emergencia */
    cancel() {
        if (appState.sos.timer) {
            clearInterval(appState.sos.timer);
            appState.sos.timer = null;
        }

        appState.sos.active = false;
        appState.event = 'LIBRE';

        const overlay = document.getElementById('sos-overlay');
        overlay.classList.add('hidden');

        speech.say('Emergencia cancelada. Todo bien.', true);
        speech.announce('Emergencia cancelada.');
        ui.showToast('Emergencia cancelada', 'info');
        utils.vibrate([100]);

        ui.updateEventBanner();
        ui.updateStatusChips();
    },
};

/* ============================================================
   MÓDULO: NAVEGACIÓN — Guía por voz
   ============================================================ */
const nav = {
    toggle() {
        appState.ui.navActive = !appState.ui.navActive;
        ui.updateNavBtn();

        if (appState.ui.navActive) {
            speech.say('Navegación asistida activada. Camina en línea recta. Te guiaré con alertas de voz.');
            speech.announce('Navegación activa.');
            ui.showToast('🧭 Navegación activa', 'success');
            utils.vibrate([100, 50, 100]);

            // Demo: instrucción aleatoria cada 20s
            nav.startInstructions();
        } else {
            speech.say('Navegación detenida.');
            speech.announce('Navegación detenida.');
            ui.showToast('Navegación detenida', 'info');
            nav.stopInstructions();
            document.getElementById('nav-instr-text').textContent = 'Presiona "Iniciar Guía" para comenzar';
            document.getElementById('nav-instr-icon').textContent = '↑';
        }
    },

    instructionTimer: null,
    instructions: [
        { icon: '↑', text: 'Continúa recto 50 metros' },
        { icon: '↰', text: 'En 20 metros, gira a la izquierda' },
        { icon: '↱', text: 'En 15 metros, gira a la derecha' },
        { icon: '⬆', text: 'Cruce de peatones al frente — espera el semáforo' },
        { icon: '✅', text: 'Llegaste a tu destino' },
    ],
    instrIndex: 0,

    startInstructions() {
        nav.instrIndex = 0;
        nav.showInstruction();
        nav.instructionTimer = setInterval(() => {
            if (!appState.ui.navActive) return;
            nav.instrIndex = (nav.instrIndex + 1) % nav.instructions.length;
            nav.showInstruction();
        }, 20000);
    },

    stopInstructions() {
        if (nav.instructionTimer) {
            clearInterval(nav.instructionTimer);
            nav.instructionTimer = null;
        }
    },

    showInstruction() {
        const instr = nav.instructions[nav.instrIndex];
        document.getElementById('nav-instr-icon').textContent = instr.icon;
        document.getElementById('nav-instr-text').textContent = instr.text;
        speech.say(instr.text);
        speech.announce(instr.text);
    },
};

/* ============================================================
   INICIALIZACIÓN — Eventos del DOM
   ============================================================ */
document.addEventListener('DOMContentLoaded', () => {

    // Cargar preferencias guardadas
    utils.loadPrefs();

    // Render inicial
    ui.renderAll();

    // Iniciar GPS del navegador
    gps.start();

    // Bienvenida por voz (con delay para que las voces se carguen)
    setTimeout(() => {
        if (appState.speech.synth) {
            // Cargar voces y luego saludar
            const sayHello = () => {
                speech.say('Centinela activo. Presiona "Conectar" para enlazar tu bastón inteligente.', false);
            };
            if (appState.speech.synth.getVoices().length > 0) {
                sayHello();
            } else {
                appState.speech.synth.addEventListener('voiceschanged', sayHello, { once: true });
            }
        }
    }, 1200);

    // ——— PESTAÑAS ———
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            ui.switchTab(btn.dataset.tab);
        });
    });

    // ——— BOTÓN SOS ———
    const sosBtn = document.getElementById('sos-btn');
    if (sosBtn) {
        let pressTimer = null;

        // Mantener presionado 1.5s para activar
        sosBtn.addEventListener('pointerdown', () => {
            pressTimer = setTimeout(() => {
                sos.activate();
            }, 1500);
            sosBtn.classList.add('sos-active');
        });

        const cancelPress = () => {
            clearTimeout(pressTimer);
            sosBtn.classList.remove('sos-active');
        };

        sosBtn.addEventListener('pointerup', cancelPress);
        sosBtn.addEventListener('pointerleave', cancelPress);

        // También al hacer click rápido (accesibilidad)
        sosBtn.addEventListener('click', () => {
            if (!appState.sos.active) {
                sos.activate();
            }
        });
    }

    // ——— CANCELAR SOS ———
    const sosCancelBtn = document.getElementById('sos-cancel-btn');
    if (sosCancelBtn) {
        sosCancelBtn.addEventListener('click', sos.cancel);
    }

    // ——— BOTÓN BT CONNECT ———
    const btConnectBtn = document.getElementById('bt-connect-btn');
    if (btConnectBtn) {
        btConnectBtn.addEventListener('click', () => {
            if (appState.bt.connected) {
                bluetooth.disconnect();
            } else {
                bluetooth.showOverlay();
            }
        });
    }

    // ——— CANCELAR OVERLAY BT ———
    const btCancelBtn = document.getElementById('bt-cancel-btn');
    if (btCancelBtn) {
        btCancelBtn.addEventListener('click', () => {
            bluetooth.hideOverlay();
            appState.bt.connecting = false;
            ui.updateBtBtn();
        });
    }

    // ——— BOTÓN NAVEGACIÓN (home) ———
    const navToggleBtn = document.getElementById('nav-toggle-btn');
    if (navToggleBtn) {
        navToggleBtn.addEventListener('click', nav.toggle);
    }

    // ——— BOTÓN NAVEGACIÓN (panel nav) ———
    const navStartBigBtn = document.getElementById('nav-start-big-btn');
    if (navStartBigBtn) {
        navStartBigBtn.addEventListener('click', nav.toggle);
    }

    // ——— DICTADO DE DESTINO ———
    const destVoiceBtn = document.getElementById('dest-voice-btn');
    if (destVoiceBtn) {
        destVoiceBtn.addEventListener('click', () => {
            const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
            if (!SpeechRecognition) {
                ui.showToast('Reconocimiento de voz no disponible', 'warning');
                return;
            }
            const rec = new SpeechRecognition();
            rec.lang = 'es-AR';
            rec.onresult = (e) => {
                const text = e.results[0][0].transcript;
                document.getElementById('destination-input').value = text;
                speech.say(`Destino: ${text}`);
            };
            rec.start();
        });
    }

    // ——— COMANDOS DE VOZ ———
    const voiceCmdBtn = document.getElementById('voice-cmd-btn');
    if (voiceCmdBtn) {
        voiceCmdBtn.addEventListener('click', () => {
            if (appState.ui.voiceCmdActive) {
                speech.stopRecognition();
            } else {
                speech.startRecognition();
            }
        });
    }

    // ——— VOLUMEN ———
    const volSlider = document.getElementById('volume-slider');
    if (volSlider) {
        volSlider.addEventListener('input', utils.debounce((e) => {
            appState.ui.volume = parseInt(e.target.value, 10);
            document.getElementById('volume-display').textContent = `${appState.ui.volume}%`;
            utils.savePrefs();
        }, 150));
    }

    // ——— ACTUALIZAR GPS ———
    const updateGpsBtn = document.getElementById('update-gps-btn');
    if (updateGpsBtn) {
        updateGpsBtn.addEventListener('click', gps.forceUpdate);
    }

    // ——— CONFIGURACIÓN: Umbral ———
    const thrSlider = document.getElementById('threshold-slider');
    if (thrSlider) {
        thrSlider.addEventListener('input', (e) => {
            appState.ui.threshold = parseInt(e.target.value, 10);
            document.getElementById('threshold-val').textContent = `${appState.ui.threshold} cm`;
            utils.savePrefs();
        });
    }

    // ——— CONFIGURACIÓN: Toggle Radar ———
    const radarToggle = document.getElementById('radar-toggle');
    if (radarToggle) {
        radarToggle.addEventListener('click', () => {
            appState.ui.radarOn = !appState.ui.radarOn;
            radarToggle.setAttribute('aria-checked', String(appState.ui.radarOn));
            const state = appState.ui.radarOn ? 'activado' : 'desactivado';
            speech.say(`Radar ${state}.`);
            ui.showToast(`Radar ${state}`, 'info');
            utils.savePrefs();
        });
    }

    // ——— CONFIGURACIÓN: Toggle Alertas de Voz ———
    const voiceAlertsToggle = document.getElementById('voice-alerts-toggle');
    if (voiceAlertsToggle) {
        voiceAlertsToggle.addEventListener('click', () => {
            appState.ui.voiceAlertsOn = !appState.ui.voiceAlertsOn;
            voiceAlertsToggle.setAttribute('aria-checked', String(appState.ui.voiceAlertsOn));
            const state = appState.ui.voiceAlertsOn ? 'activadas' : 'desactivadas';
            if (appState.ui.voiceAlertsOn) speech.say(`Alertas de voz ${state}.`);
            ui.showToast(`Alertas de voz ${state}`, 'info');
            utils.savePrefs();
        });
    }

    // ——— CONFIGURACIÓN: Guardar Contacto ———
    const saveContactBtn = document.getElementById('save-contact-btn');
    if (saveContactBtn) {
        saveContactBtn.addEventListener('click', () => {
            appState.contact.name  = document.getElementById('contact-name').value.trim();
            appState.contact.phone = document.getElementById('contact-phone').value.trim();

            // Actualizar el link de llamada
            const callBtn = document.getElementById('call-contact-btn');
            if (callBtn) callBtn.href = `tel:${appState.contact.phone}`;

            utils.savePrefs();
            speech.say(`Contacto guardado: ${appState.contact.name}.`);
            ui.showToast(`✅ Contacto guardado: ${appState.contact.name}`, 'success');
        });
    }

    // ——— CONFIGURACIÓN: Tamaño de fuente (botones) ———
    document.querySelectorAll('.font-size-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            ui.applyFontSize(btn.dataset.size);
            speech.say(`Texto tamaño ${btn.dataset.size}.`);
        });
    });

    // ——— HEADER: botones de fuente ———
    document.getElementById('font-increase')?.addEventListener('click', () => {
        const sizes = ['small', 'normal', 'large', 'xlarge'];
        const cur = sizes.indexOf(appState.ui.fontSize);
        if (cur < sizes.length - 1) ui.applyFontSize(sizes[cur + 1]);
        else ui.showToast('Tamaño máximo alcanzado', 'info');
    });

    document.getElementById('font-decrease')?.addEventListener('click', () => {
        const sizes = ['small', 'normal', 'large', 'xlarge'];
        const cur = sizes.indexOf(appState.ui.fontSize);
        if (cur > 0) ui.applyFontSize(sizes[cur - 1]);
        else ui.showToast('Tamaño mínimo alcanzado', 'info');
    });

    // ——— TECLADO: Accesos directos ———
    document.addEventListener('keydown', (e) => {
        if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA') return;

        switch (e.key) {
            case 'Escape':
                if (appState.sos.active) sos.cancel();
                break;
            case 's':
            case 'S':
                e.preventDefault();
                sos.activate();
                break;
            case 'v':
            case 'V':
                e.preventDefault();
                if (appState.ui.voiceCmdActive) speech.stopRecognition();
                else speech.startRecognition();
                break;
            case 'c':
            case 'C':
                e.preventDefault();
                if (!appState.bt.connected) bluetooth.showOverlay();
                else bluetooth.disconnect();
                break;
            case 'n':
            case 'N':
                e.preventDefault();
                nav.toggle();
                break;
            case '1': ui.switchTab('home'); break;
            case '2': ui.switchTab('nav'); break;
            case '3': ui.switchTab('map'); break;
            case '4': ui.switchTab('config'); break;
        }
    });

    // ——— CERRAR OVERLAY CON ESC ———
    document.getElementById('bt-overlay')?.addEventListener('click', (e) => {
        if (e.target === document.getElementById('bt-overlay')) {
            bluetooth.hideOverlay();
            appState.bt.connecting = false;
            ui.updateBtBtn();
        }
    });

    // ——— TOQUE LARGO en el botón SOS del overlay ———
    document.getElementById('sos-call-btn')?.addEventListener('click', () => {
        clearInterval(appState.sos.timer);
        speech.say(`Llamando a ${appState.contact.name}.`, true);
    });

    // ——— ACCESSIBILITY: Anunciar pestaña al cambiar ———
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            const tabNames = { home: 'Inicio', nav: 'Navegación', map: 'Mapa GPS', config: 'Configuración' };
            speech.announce(`Pestaña ${tabNames[btn.dataset.tab] || btn.dataset.tab}`);
        });
    });

    console.log(
        '%cCENTINELA v2.0 — Bastón Inteligente\n' +
        '%c✅ App lista | WCAG 2.1 AA | Web Speech API | GPS activo\n' +
        'Atajos: [S] SOS | [V] Voz | [C] Conectar | [N] Navegar | [1-4] Pestañas',
        'color:#C5A059;font-size:1.2em;font-weight:bold;',
        'color:#D8C5A5;font-size:0.9em;'
    );
});
