# 🛡️ CENTINELA v2.0 — MAPA DEFINITIVO DE HARDWARE Y ARQUITECTURA

**Bastón Inteligente de Navegación Segura y Asistencia para Personas con Discapacidad Visual**  
*Versión de Firmware:* 2.0 Definitiva  
*Plataforma:* ESP32-WROOM-32 (240 MHz, Dual Core, Bluetooth SPP + I2S Audio)  
*Compatibilidad Core:* `arduino-esp32` v2.0.x y v3.x  

---

## 1. 📌 MAPA DEFINITIVO DE PINES GPIO (ESP32-WROOM-32)

> **Criterios de ingeniería aplicados:**
> - Se eliminó todo conflicto de hardware (el pin GPIO 39 quedó exclusivo para el micrófono analógico; el eco del radar se movió a GPIO 36).
> - Se respetan los pines **INPUT-ONLY** del ESP32 (GPIO 34, 35, 36, 39): se usan exclusivamente para recepciones (`ECHO` y `ADC1`).
> - Se utiliza exclusivamente el conversor **ADC1** (GPIO 32 a 39) para mediciones analógicas, garantizando que el **Bluetooth activo NO inhabilite el ADC** (el ADC2 se desactiva al usar WiFi/Bluetooth).
> - Se aíslan las cargas de potencia (motores vibradores y parlante) mediante MOSFETs y regulador step-down independiente.

| GPIO | Componente Físico | Función / Señal | Modo ESP32 | Circuito / Notas de Hardware |
| :---: | :--- | :--- | :---: | :--- |
| **GPIO 2** | HC-SR04 Radar | `TRIG_RADAR` | `OUTPUT` | Pulso 10 µs. Resistencia pull-down 10 kΩ externa. |
| **GPIO 4** | Motor Izquierdo | `MOTOR_IZQUIERDO` | `OUTPUT PWM` | LEDC Ch 0 / 5 kHz 8-bit. Gate de MOSFET Canal N (2N7002 / IRLML2502). |
| **GPIO 5** | Motor Central | `MOTOR_CENTRO` | `OUTPUT PWM` | LEDC Ch 1 / 5 kHz 8-bit. Alerta de pozos y frontal. Gate de MOSFET Canal N. |
| **GPIO 13** | Servomotor SG90 | `PIN_SERVO` | `OUTPUT PWM` | Señal PWM 50 Hz. Alimentación obligatoria desde rail de 5V (no 3.3V). |
| **GPIO 14** | Jack 3.5mm TRRS | `PIN_JACK_DETECT` | `INPUT_PULLUP` | Detección mecánica: `LOW` = auricular enchufado (silencia parlante). |
| **GPIO 15** | Pulsador SOS | `PIN_BOTON_SOS` | `INPUT_PULLUP` | Pulsador táctil a GND. Antirrebote por software (50 ms). `LOW` = presionado. |
| **GPIO 16** | GPS NEO-6M | `GPS_RX_PIN` | `UART2 RX` | Conectar al pin `TX` del módulo NEO-6M (9600 baudios, 8N1). |
| **GPIO 17** | GPS NEO-6M | `GPS_TX_PIN` | `UART2 TX` | Opcional (para enviar comandos de configuración al GPS). |
| **GPIO 18** | Motor Derecho | `MOTOR_DERECHO` | `OUTPUT PWM` | LEDC Ch 2 / 5 kHz 8-bit. Gate de MOSFET Canal N. |
| **GPIO 19** | HC-SR04 Suelo | `TRIG_SUELO` | `OUTPUT` | Detección de pozos y bajadas de cordón/escaleras. |
| **GPIO 21** | MPU-6050 | `MPU_SDA` | `I2C DATA` | Bus I2C a 400 kHz. Pull-ups de 4.7 kΩ a 3.3V en el módulo. |
| **GPIO 22** | MPU-6050 | `MPU_SCL` | `I2C CLOCK` | Reloj I2C. |
| **GPIO 23** | HC-SR04 Frontal | `TRIG_FRONTAL` | `OUTPUT` | Obstáculos torso y cabeza. |
| **GPIO 25** | MAX98357A | `I2S_WS_PIN` | `I2S LRC` | Word Select / LRCLK (16 kHz). |
| **GPIO 26** | MAX98357A | `I2S_BCK_PIN` | `I2S BCLK` | Bit Clock continuo. |
| **GPIO 27** | MAX98357A | `I2S_DATA_PIN`| `I2S DIN` | Datos de audio digital PCM. |
| **GPIO 32** | MAX98357A | `I2S_SD_PIN` | `OUTPUT` | Shutdown amplificador: `HIGH` = Activo, `LOW` = Mute / Ahorro energía. |
| **GPIO 33** | Batería 2S (ADC) | `PIN_BATERIA` | `ADC1_CH5` | Divisor resistivo 100 kΩ / 47 kΩ (mapea 8.4V → 2.69V). |
| **GPIO 34** | HC-SR04 Frontal | `ECHO_FRONTAL` | `INPUT (solo)` | Retorno acústico frontal. Divisor 5V→3.3V (1kΩ/2kΩ) o directo 3.3V. |
| **GPIO 35** | HC-SR04 Suelo | `ECHO_SUELO` | `INPUT (solo)` | Retorno acústico suelo. |
| **GPIO 36** | HC-SR04 Radar | `ECHO_RADAR` | `INPUT (solo)` | Retorno acústico radar rotativo (reubicado para liberar GPIO 39). |
| **GPIO 39** | Micrófono Electret| `PIN_MICROFONO` | `ADC1_CH3` | Entrada analógica exclusiva con desacople AC para captura de voz (VAD). |

---

## 2. ⚡ SISTEMA DE ENERGÍA Y POTENCIA

```
   [2x Baterías 18650 Li-Ion en Serie (7.4V nominal / 8.4V cargadas)]
                               │
                ┌──────────────┴──────────────┐
                ▼                             ▼
       [Divisor 100k/47k]           [Interruptor General + Fusible 2A]
                │                                     │
                ▼                                     ▼
        ESP32 GPIO 33 (ADC)                [Step-Down MP1584 / LM2596]
                                          (Regulado a 5.0V / 3A estables)
                                                      │
                       ┌──────────────────────────────┼──────────────────────────────┐
                       ▼                              ▼                              ▼
                 [ESP32 Vin/5V]                  [Servo SG90]                  [3x Motores ERM]
             (LDO interno a 3.3V)           (Rail 5V independiente)         (Drain MOSFET a 5V)
                       │                              │                              │
         ┌─────────────┴─────────────┐         [C = 470 µF]                    [Diodos 1N4148]
         ▼                           ▼      (filtra picos inductivos)      (protección flyback)
   [I2C MPU6050]              [HC-SR04 x3]
    [3.3V LDO]                 [5V o 3.3V]
```

---

## 3. 🚶 RESOLUCIÓN DE SITUACIONES CRÍTICAS EN LA CALLE

### A. Autonomía Absoluta del Bastón
* **Problema teórico:** El bastón depende de una app en el celular.
* **Solución real implementada:** El firmware opera de forma **100% autónoma**. Si el teléfono se queda sin batería, se apaga o pierde el Bluetooth, los sensores ultrasónicos, el radar rotativo, la detección de caídas y los motores hápticos siguen guiando al usuario con total precisión.

### B. Discriminación entre Golpeteo del Bastón y Caída Real
* **Problema teórico:** El bastón choca contra el piso constantemente en el caminar ciego (técnica de toque rítmico); un acelerómetro simple interpretaría cada toque como una caída.
* **Solución real implementada:** Máquina de estados en dos fases:
  1. Para que se considere caída, el sensor debe registrar primero **ingravidez / caída libre (< 0.45 G)** durante al menos 80–150 ms.
  2. Luego, dentro de una ventana máxima de 800 ms, debe registrarse el **impacto (> 1.75 G)**.
  El golpeteo de marcha tiene deceleración brusca pero **nunca ingravidez previa**, eliminando los falsos positivos.

### C. Retención de Coordenadas ante Caída de Señal GPS
* **Problema teórico:** Al entrar bajo una marquesina, túnel o negocio techado, el GPS pierde los satélites y emitiría `(0.0, 0.0)`.
* **Solución real implementada:** En `GPS.cpp`, cuando `Fix == false`, **las últimas coordenadas válidas conocidas se preservan intactas en memoria**. Si el usuario presiona SOS o sufre una caída en el túnel, la app y el sistema rescatan la última posición conocida con certeza.

### D. Primacía Háptica contra el Ruido Urbano
* **Problema teórico:** En esquinas con tránsito pesado, colectivos o lluvia, el audio puede ser inaudible.
* **Solución real implementada:** La mano del usuario recibe información táctil direccional en todo momento:
  - **Vibrador Izquierdo:** Alerta lateral izquierda.
  - **Vibrador Central:** Obstáculo frontal o pozo.
  - **Vibrador Derecho:** Alerta lateral derecha.
  - **Patrón Intermitente Máximo:** Emergencia / Caída.

### E. Detección Temprana de Pozos y Desniveles Negativos
* **Problema teórico:** Un pozo o fin de vereda no bloquea el paso hacia adelante, pero causa caídas graves.
* **Solución real implementada:** El sensor `HC-SR04 Suelo` mide constantemente la distancia al pavimento (~15–20 cm en marcha normal). Si la distancia supera los 60 cm, `Decision` clasifica el evento como `CRITICO`, activando el motor central con el patrón de pozo (`PWM_POZO`).

---

## 4. 📲 INTEROPERABILIDAD TOTAL: APP ANDROID (KIVY) + APP WEB

El firmware implementado en `Bluetooth.cpp` habla de forma simultánea los dos dialectos de telemetría:
1. **App Nativa Android (`Centinela_App`):**
   - Emite: `EVENTO:SOS`, `EVENTO:CAIDA`, `EVENTO:PELIGRO`, `EVENTO:NORMAL`, `BATERIA:xx`.
   - Streaming de audio: Cabecera mágica `[0xA5, 0x5A]` de 16 kHz 16-bit mono.
   - Comandos: `B:1` / `B:0` (baliza acústica), `SOS:1` / `SOS:0`.
2. **App Web Accesible (`presentacion/`):**
   - Recibe la telemetría extendida: `ALERTA:x`, `ZONA:x`, `FRONTAL:x`, `SUELO:x`, `RADAR:x`, `GPS:lat,lon,fix`.
   - Síntesis de voz Web Speech en español accesible WCAG 2.1 AA.
   - Botón SOS central de alto contraste con llamada directa y enlace a Google Maps.

---

## 5. 🛠️ GUÍA DE COMPILACIÓN (ARDUINO IDE)

1. Abrir `Centinela_v2.ino`.
2. Seleccionar la placa: **Tools → Board → ESP32 Arduino → ESP32 Dev Module**.
3. Parámetros de placa:
   - **CPU Frequency:** `240MHz (WiFi/BT)`
   - **Flash Frequency:** `80MHz`
   - **Flash Mode:** `QIO`
   - **Flash Size:** `4MB (32Mb)`
   - **Partition Scheme:** `Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)`
   - **Upload Speed:** `921600`
4. Instalar librerías en el Library Manager:
   - `ESP32Servo` (por Kevin Harrington)
   - `MPU6050_light` (por rfetick)
5. Compilar y cargar.
