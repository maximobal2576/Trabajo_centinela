# 🦯 CENTINELA v2.0 — Guía de Exposición

## Bastón Inteligente Asistivo con ESP32

**Versión:** 2.0  
**Plataforma:** ESP32-WROOM-32 / Arduino IDE  
**Autores:** Equipo Centinela  
**Fecha:** Septiembre 2026

---

## PÁGINA 1 — Introducción y Visión del Proyecto

### ¿Qué es Centinela?

**Centinela** es un bastón inteligente diseñado para asistir a personas con discapacidad
visual en su movilidad diaria. Utiliza un ESP32 como cerebro central para integrar
**15 componentes de hardware** que trabajan en conjunto para detectar obstáculos,
desniveles, caídas y emergencias, comunicando todo al usuario mediante vibraciones
hápticas, audio y una aplicación móvil por Bluetooth.

### Problema que resuelve

Según la OMS, más de **2.200 millones de personas** en el mundo tienen alguna
deficiencia visual. Los bastones blancos tradicionales solo permiten detectar
obstáculos por contacto directo. Centinela **amplía el rango de detección** a
80 cm usando ultrasonido, detecta desniveles en el suelo (pozos, escalones),
sensa caídas del usuario y permite enviar alertas de emergencia con ubicación GPS.

### Objetivo general

Desarrollar un dispositivo wearable de bajo costo que mejore significativamente
la autonomía y seguridad de personas con discapacidad visual, integrando
sensores, actuadores y comunicación inalámbrica en un bastón convencional.

---

## PÁGINA 2 — Arquitectura General del Sistema

### Diagrama de bloques

```
┌─────────────────────────────────────────────────────────┐
│                    ESP32-WROOM-32                        │
│                   (240 MHz, WiFi+BT)                    │
│                                                         │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐              │
│  │ Sensores │  │  Radar   │  │Movimiento│              │
│  │ HC-SR04  │  │Servo+US  │  │ MPU-6050 │              │
│  │ x3       │  │ SG90     │  │ I2C      │              │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘              │
│       │              │              │                    │
│       ▼              ▼              ▼                    │
│  ┌─────────────────────────────────────┐                │
│  │         Motor de Decisión           │                │
│  │   (Prioridades + Lógica fuzzy)      │                │
│  └──────────────┬──────────────────────┘                │
│                 │                                        │
│       ┌─────────┼─────────┐                             │
│       ▼         ▼         ▼                             │
│  ┌────────┐ ┌────────┐ ┌────────┐                      │
│  │Motor   │ │Motor   │ │Motor   │                      │
│  │Izq PWM │ │Ctr PWM │ │Der PWM │                      │
│  └────────┘ └────────┘ └────────┘                      │
│                                                         │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐              │
│  │Bluetooth │  │Audio I2S │  │   GPS    │              │
│  │  SPP     │  │MAX98357A │  │ NEO-6M   │              │
│  │  ◄────►  │  │+ TRRS   │  │ UART     │              │
│  └──────────┘  └──────────┘  └──────────┘              │
│                                                         │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐              │
│  │Micrófono │  │Emergencia│  │ Batería  │              │
│  │ ADC      │  │Botón SOS │  │ 2x18650  │              │
│  └──────────┘  └──────────┘  └──────────┘              │
└─────────────────────────────────────────────────────────┘
```

### Flujo de datos principal

1. **Entrada:** Sensores ultrasónicos + MPU-6050 + GPS + Micrófono + Botón SOS
2. **Procesamiento:** Motor de decisión con prioridades (Emergencia > Caída > Pozo > Obstáculo)
3. **Salida:** Motores vibradores (háptico) + Audio I2S (parlante/auriculares) + Bluetooth (app)

---

## PÁGINA 3 — Lista Completa de Componentes

| # | Componente | Función | Pin(es) ESP32 |
|---|-----------|---------|---------------|
| 1 | ESP32-WROOM-32 | Microcontrolador central 240MHz | — |
| 2 | HC-SR04 Frontal | Detección de obstáculos al frente | TRIG: 23, ECHO: 35 |
| 3 | HC-SR04 Suelo | Detección de desniveles/pozos | TRIG: 19, ECHO: 36 |
| 4 | HC-SR04 Radar | Barrido angular izq-centro-der | TRIG: 2, ECHO: 39 |
| 5 | MPU-6050 | Acelerómetro/giroscopio (caídas) | SDA: 21, SCL: 22 |
| 6 | Servomotor SG90 | Radar de barrido angular 45°-135° | Pin: 13 |
| 7 | Motor vibrador Izq | Feedback háptico izquierda | Pin: 4 (LEDC CH0) |
| 8 | Motor vibrador Centro | Feedback háptico centro | Pin: 5 (LEDC CH1) |
| 9 | Motor vibrador Der | Feedback háptico derecha | Pin: 18 (LEDC CH2) |
| 10 | 3x MOSFET canal N | Drivers de potencia 3.3V→5V | — |
| 11 | MAX98357A | DAC de audio I2S mono 3W | BCK:26, WS:25, DATA:27, SD:32 |
| 12 | Parlante 4Ω/3W | Reproducción de audio | Via MAX98357A |
| 13 | TRRS Breakout | Jack 3.5mm auricular+micrófono | Detect: 33 |
| 14 | Micrófono electret | Captura de voz del usuario | Pin: 39 (ADC1) |
| 15 | GPS NEO-6M | Geolocalización NMEA | RX2: 16, TX2: 17 |
| 16 | Pulsador SOS | Emergencia instantánea | Pin: 15 (PULLUP) |
| 17 | 2x Batería 18650 | Alimentación 7.4V nominal | ADC: 34 |
| 18 | Regulador Step-Down | Conversión a 5V/3.3V | — |
| 19 | Fusible + Interruptor | Protección eléctrica | — |

**Costo estimado total:** ~$35-45 USD

---

## PÁGINA 4 — Mapa de Pines del ESP32

```
    ┌──────────────────────────────┐
    │       ESP32 DevKit 38pin     │
    │                              │
    │  GPIO 23 ─── TRIG Frontal   │
    │  GPIO 35 ─── ECHO Frontal*  │
    │  GPIO 19 ─── TRIG Suelo     │
    │  GPIO 36 ─── ECHO Suelo*    │
    │  GPIO  2 ─── TRIG Radar     │
    │  GPIO 39 ─── ECHO Radar*    │
    │                              │
    │  GPIO 21 ─── SDA (MPU-6050) │
    │  GPIO 22 ─── SCL (MPU-6050) │
    │                              │
    │  GPIO 13 ─── Servo SG90     │
    │                              │
    │  GPIO  4 ─── Motor Izq (MOSFET) │
    │  GPIO  5 ─── Motor Centro (MOSFET)│
    │  GPIO 18 ─── Motor Der (MOSFET)  │
    │                              │
    │  GPIO 26 ─── I2S BCK        │
    │  GPIO 25 ─── I2S WS/LRC     │
    │  GPIO 27 ─── I2S DATA/DIN   │
    │  GPIO 32 ─── I2S SD_MODE    │
    │  GPIO 33 ─── Jack Detect    │
    │                              │
    │  GPIO 16 ─── GPS RX2        │
    │  GPIO 17 ─── GPS TX2        │
    │                              │
    │  GPIO 15 ─── Botón SOS      │
    │  GPIO 34 ─── ADC Batería*   │
    │                              │
    │  * = Input-only con divisor  │
    │      de tensión (5V → 3.3V) │
    └──────────────────────────────┘
```

**Nota importante:** Los GPIOs 34, 35, 36 y 39 son **solo entrada** (input-only).
Se usan para los ECHO de los HC-SR04 y el ADC de batería, conectados mediante
divisores resistivos de tensión (R1=1kΩ, R2=2kΩ) para proteger al ESP32.

---

## PÁGINA 5 — Estructura del Software (Arquitectura Modular)

### Organización de archivos

```
Centinela_v2/
├── Centinela_v2.ino      ← Archivo principal (setup + loop)
├── Configuracion.h       ← Pines, constantes y tiempos
├── Tipos.h               ← Enums y structs de datos
│
├── Estado.h / .cpp       ← Estado global compartido (singleton)
├── Scheduler.h / .cpp    ← Planificador cooperativo sin delay()
│
├── Sensores.h / .cpp     ← HC-SR04 frontal y suelo
├── Radar.h / .cpp        ← Servo SG90 + HC-SR04 barrido angular
├── Movimiento.h / .cpp   ← MPU-6050 detección de caídas
│
├── Decision.h / .cpp     ← Lógica de toma de decisiones
├── Salida.h / .cpp       ← Control PWM de motores vibradores
│
├── Bluetooth.h / .cpp    ← Comunicación SPP (texto + audio PCM)
├── Audio.h / .cpp        ← I2S MAX98357A + conmutación TRRS
├── Microfono.h / .cpp    ← Captura de voz VAD + transmisión
├── GPS.h / .cpp          ← Parser NMEA del NEO-6M
│
├── Emergencia.h / .cpp   ← Botón SOS con debouncing
├── Bateria.h / .cpp      ← Telemetría ADC de batería
├── Diagnostico.h / .cpp  ← Monitor serial USB
│
└── GUIA_EXPOSICION.md    ← Esta guía
```

### Patrón de diseño

Cada módulo sigue el patrón **Init-Update**:
- `iniciar()` → Se llama una vez en `setup()`, configura hardware
- `actualizar()` → Se llama periódicamente desde el `Scheduler`

El `Scheduler` es un **planificador cooperativo** que ejecuta cada módulo
a intervalos específicos sin usar `delay()`, garantizando respuesta en tiempo real.

---

## PÁGINA 6 — El Planificador Cooperativo (Scheduler)

### ¿Por qué no usar delay()?

`delay()` bloquea completamente el procesador. Si el sensor frontal tarda 15ms
en responder, durante ese tiempo no se puede leer el botón de emergencia.
El Scheduler resuelve esto ejecutando cada tarea solo cuando su temporizador expira.

### Tabla de tiempos

| Módulo | Intervalo (ms) | Prioridad | Justificación |
|--------|:--------------:|-----------|---------------|
| Emergencia | 50 | Crítica | Respuesta inmediata al botón SOS |
| Micrófono | Cada loop | Alta | Muestreo continuo de voz |
| Movimiento (MPU) | 20 | Alta | Detección rápida de caídas |
| Decisión | 20 | Alta | Actualizar vibraciones sin latencia |
| Salida (Motores) | 20 | Alta | PWM suave sin parpadeo |
| Audio I2S | 20 | Alta | Streaming sin cortes |
| Sensores (US) | 40 | Media | HC-SR04 necesita ~15ms por medición |
| Radar (Servo) | 60 | Media | Barrido mecánico gradual |
| Bluetooth | 150 | Normal | Telemetría no requiere velocidad |
| GPS | 200 | Normal | NEO-6M envía datos a 1Hz |
| Batería | 4000 | Baja | Voltaje cambia lentamente |
| Diagnóstico | 1000 | Debug | Solo en modo desarrollo |

### Funcionamiento del loop principal

```cpp
void loop() {
    Scheduler::actualizar(); // ← Una sola línea, ejecuta todo
}
```

Internamente, `Scheduler::actualizar()` compara `millis()` con el último
tiempo de ejecución de cada módulo y lo ejecuta solo cuando corresponde.

---

## PÁGINA 7 — Sistema de Detección de Obstáculos

### Sensor Ultrasónico HC-SR04

**Principio:** Emite un pulso ultrasónico de 40kHz, mide el tiempo de vuelo
del eco y calcula la distancia: `distancia = (tiempo × 0.034) / 2`

**Configuración de 3 sensores:**

| Sensor | Apunta hacia | Detecta |
|--------|-------------|---------|
| Frontal | Adelante horizontal | Obstáculos al frente (paredes, postes) |
| Suelo | Hacia abajo | Pozos, escalones, desniveles |
| Radar | Angular (servo) | Obstáculos laterales izq/centro/der |

### Radar de Barrido Angular

El servomotor SG90 gira el sensor de radar entre **45° y 135°** en pasos
de 5°, creando un "barrido" que cubre el frente del usuario en 3 zonas:

```
           Izquierda    Centro    Derecha
              45°         90°       135°
               \          |          /
                \         |         /
                 \   ZONA | ZONA   /
                  \  IZQ  | DER   /
                   \      |      /
                    \     |     /
                     \    |    /
                      \   |   /
                       \  |  /
                        USUARIO
```

Cada zona activa un motor vibrador diferente para indicar la dirección del obstáculo.

---

## PÁGINA 8 — Sistema de Vibración Háptica

### Principio de feedback háptico

El usuario siente vibraciones de diferente intensidad en 3 puntos de contacto
del bastón, indicándole de dónde viene el obstáculo y qué tan cerca está.

### Escala de intensidad PWM

| Nivel | Valor PWM | Rango de distancia | Significado |
|-------|:---------:|:------------------:|-------------|
| Suave | 80 | 60-80 cm | Obstáculo lejano, precaución |
| Medio | 150 | 40-60 cm | Obstáculo detectado, reducir velocidad |
| Fuerte | 210 | 20-40 cm | Obstáculo cercano, cambiar dirección |
| Máximo | 255 | 0-20 cm | Peligro inmediato, detenerse |
| Pozo | 130 | Suelo > 80 cm | Desnivel en el piso detectado |

### Cálculo de intensidad (proporcional a la distancia)

```
Intensidad = map(distancia, 80cm, 20cm, PWM_SUAVE, PWM_MAXIMO)
```

Cuanto más cerca el obstáculo → más fuerte la vibración.

### Patrones especiales

- **Emergencia/Caída:** Los 3 motores pulsan ON/OFF cada 200ms (patrón de alarma)
- **Pozo detectado:** Solo el motor central vibra a PWM_POZO (130)

---

## PÁGINA 9 — Detección de Caídas (MPU-6050)

### Funcionamiento del acelerómetro

El MPU-6050 mide la aceleración en 3 ejes (X, Y, Z). En reposo vertical,
la magnitud del vector de aceleración es ≈ 1G (9.81 m/s²).

### Algoritmo de detección

Se calcula el módulo del vector de aceleración:

$$\text{módulo} = \sqrt{X^2 + Y^2 + Z^2}$$

| Condición | Módulo (G) | Significado |
|-----------|:----------:|-------------|
| módulo < 0.15 | Lectura inválida | Sensor desconectado, ignorar |
| módulo < 0.45 | **Caída libre** | El bastón está cayendo |
| 0.45 ≤ módulo ≤ 1.75 | Normal | Uso habitual del bastón |
| módulo > 1.75 | **Golpe fuerte** | Impacto contra superficie |

### Respuesta ante caída

1. Todos los motores vibran a máxima intensidad (PWM 255)
2. Se notifica por Bluetooth a la app
3. El sistema entra en estado `SISTEMA_CAIDA`
4. La app puede llamar automáticamente a un contacto de emergencia

---

## PÁGINA 10 — Sistema de Audio (MAX98357A + TRRS)

### Arquitectura de audio

```
    App Celular                           Bastón
    ┌─────────┐     Bluetooth SPP     ┌─────────────┐
    │ Audio   │ ──── PCM 16bit ─────► │ Buffer      │
    │ TTS     │     (0xA5 0x5A)       │ Circular    │
    └─────────┘                       └──────┬──────┘
                                             │
                                        ┌────▼────┐
                                        │  I2S    │
                                        │  Bus    │
                                        └──┬───┬──┘
                                           │   │
                                    ┌──────┘   └──────┐
                                    ▼                  ▼
                             ┌──────────┐      ┌──────────┐
                             │MAX98357A │      │  TRRS    │
                             │Amplific. │      │  Jack    │
                             │ 3W       │      │ 3.5mm   │
                             └────┬─────┘      └────┬─────┘
                                  ▼                  ▼
                             ┌─────────┐      ┌──────────┐
                             │Parlante │      │Auricular │
                             │ 4Ω/3W   │      │ 3.5mm    │
                             └─────────┘      └──────────┘
```

### Conmutación inteligente TRRS

- **Sin auriculares:** Audio sale por el parlante (SD_MODE = HIGH)
- **Con auriculares:** Parlante se apaga 100% (SD_MODE = LOW), audio solo por auriculares
- **Sin audio:** Amplificador en shutdown total (<0.6µA) para ahorrar batería

### Protocolo de audio por Bluetooth

```
Trama de audio: [0xA5] [0x5A] [len_lo] [len_hi] [datos PCM...]
```

- Magic bytes: `0xA5 0x5A` identifican un paquete de audio
- Los siguientes 2 bytes son la longitud del chunk (little-endian)
- Los datos son muestras PCM signed 16-bit mono a 16kHz

---

## PÁGINA 11 — Sistema de Micrófono y Captura de Voz

### Detección de Actividad Vocal (VAD)

El micrófono se muestrea continuamente por ADC. Cuando la amplitud supera
el umbral de voz (200), se inicia la grabación automática.

### Flujo de captura

```
                    Silencio           Voz detectada        Silencio > 700ms
                       │                   │                      │
    [INACTIVO] ────────┤                   │                      │
                       │         ┌─────────▼──────────┐           │
                       │         │   MIC_START         │           │
                       │         │   (BT → App)        │           │
                       │         └─────────┬──────────┘           │
                       │                   │                      │
                       │         ┌─────────▼──────────┐           │
                       │         │   Muestreo ADC      │           │
                       │         │   PCM 16-bit        │           │
                       │         │   Buffer 256 bytes  │◄──── Repite
                       │         └─────────┬──────────┘           │
                       │                   │ Buffer lleno         │
                       │         ┌─────────▼──────────┐           │
                       │         │   MIC_CHUNK:base64  │           │
                       │         │   (BT → App)        │           │
                       │         └─────────┬──────────┘           │
                       │                   │                      │
                       │         ┌─────────▼──────────┐           │
                       │         │   MIC_STOP          │◄──────────┘
                       │         │   (BT → App)        │
                       │         └────────────────────┘
```

### Protocolo de micrófono

```
MIC_START              → Indica inicio de grabación
MIC_CHUNK:<base64>     → Datos PCM codificados en Base64
MIC_STOP               → Indica fin de grabación
```

La app recibe estos datos, los decodifica y puede usar Speech-to-Text
para ejecutar comandos de voz (navegación, llamadas, etc.).

---

## PÁGINA 12 — Sistema GPS (NEO-6M)

### Conexión al ESP32

El GPS NEO-6M se conecta al **Serial2 por hardware** del ESP32:
- ESP32 GPIO 16 (RX2) ← TX del GPS
- ESP32 GPIO 17 (TX2) → RX del GPS
- Baudrate: 9600

### Parseo de sentencias NMEA

El módulo procesa sentencias `$GPRMC` y `$GNRMC` que contienen:

```
$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
       │       │ │         │ │         │
       Hora    │ Latitud   │ Longitud  Velocidad
               │           │
        A=Válido        N/S/E/W
        V=Sin fix       hemisferio
```

### Conversión NMEA → Grados decimales

```
Formato NMEA: ddmm.mmmm
Ejemplo: 4807.038 = 48° 07.038'

Conversión:
  grados = 48
  minutos = 07.038
  decimal = 48 + (7.038 / 60) = 48.1173°
```

### Uso de coordenadas

Las coordenadas se envían por Bluetooth en formato:
```
GPS:<latitud>,<longitud>,<fijado>
```

La app puede usar estas coordenadas para:
- Navegación guiada por voz
- Envío de ubicación en emergencias (SOS)
- Registro de recorridos

---

## PÁGINA 13 — Sistema de Emergencia (SOS)

### Botón físico de emergencia

Un pulsador conectado entre GPIO 15 y GND (con `INPUT_PULLUP`) permite al usuario
activar una alerta de emergencia instantánea.

### Debouncing por software

Los pulsadores mecánicos generan "rebotes" eléctricos al presionarlos.
El módulo Emergencia implementa debouncing de 50ms:

```
Señal real del botón:
    ──┐ ┌┐ ┌┐ ┌──────────────────── (presionado)
      └─┘ └─┘
    ◄─ rebote ─►

Señal filtrada (debounce 50ms):
    ────────────┐
                └──────────────────── (confirmado)
    ◄── 50ms ──►
```

### Secuencia de activación SOS

1. Usuario presiona el botón
2. Debounce confirma la pulsación (50ms)
3. Sistema entra en `SISTEMA_EMERGENCIA` con `PRIORIDAD_CRITICA`
4. Los 3 motores vibran en patrón de alarma
5. Se envía `EVENTO:EMERGENCIA` y `SOS:1` por Bluetooth
6. La app activa protocolo de emergencia:
   - Llamada al 911 o contacto de confianza
   - Envío de coordenadas GPS
   - Alerta sonora

---

## PÁGINA 14 — Sistema de Batería y Gestión de Energía

### Alimentación del sistema

```
    2x Batería 18650 (Serie)
    ┌────┬────┐
    │7.4V│8.4V│  ← Nominal / Carga completa
    │nom │max │
    └──┬──┘
       │
       ▼
    Regulador Step-Down
    ┌──────────┐
    │  5V / 3A │──── Sensores + Servomotor + Motores
    │  3.3V    │──── ESP32 + GPS
    └──────────┘
```

### Medición por ADC

El voltaje de batería se mide mediante un divisor resistivo:

```
VBat ─── R1(100kΩ) ─── GPIO 34 ─── R2(33kΩ) ─── GND
                         │
                    V_pin = VBat × 33/(100+33)
                    A 8.4V: V_pin = 2.08V (seguro)
```

### Filtro exponencial

Para evitar lecturas ruidosas, se aplica un filtro de suavizado:

```
lectura_filtrada = (lectura_anterior × 7 + lectura_nueva) / 8
```

### Umbrales de batería

| Porcentaje | Estado | Acción |
|:----------:|--------|--------|
| > 18% | Normal | Funcionamiento completo |
| < 15% | Batería baja | Evento BATERIA_BAJA, notificación a app |
| ≥ 18% (recuperación) | Normal | Regresa a estado normal |

---

## PÁGINA 15 — Comunicación Bluetooth SPP

### Protocolo de comunicación

Centinela usa **Bluetooth Serial Port Profile (SPP)** para comunicarse
con la app Android. El protocolo es **dual**:

1. **Texto:** Comandos y telemetría separados por `\n`
2. **Audio:** Paquetes PCM con header mágico `0xA5 0x5A`

### Telemetría enviada (Bastón → App)

| Campo | Formato | Ejemplo |
|-------|---------|---------|
| EVENTO | `EVENTO:<n>` | `EVENTO:1` (obstáculo frontal) |
| DIRECCION | `DIRECCION:<n>` | `DIRECCION:4` (derecha) |
| BATERIA | `BATERIA:<n>` | `BATERIA:78` |
| AUDIO | `AUDIO:<0/1>` | `AUDIO:1` (streaming) |
| BLUETOOTH | `BLUETOOTH:<0/1>` | `BLUETOOTH:1` |
| ESTADO | `ESTADO:<n>` | `ESTADO:1` (normal) |
| AURICULAR | `AURICULAR:<0/1>` | `AURICULAR:0` |
| GPS | `GPS:<lat>,<lon>,<fix>` | `GPS:-34.603722,-58.381592,1` |

### Comandos recibidos (App → Bastón)

| Comando | Descripción |
|---------|-------------|
| `PING` | Prueba de conexión (responde `PONG`) |
| `TEST:<pwm>` | Prueba de motores (0-255) |
| `N:<0/1>` | Activa/desactiva navegación |
| `G:<0/1>` | GPS disponible en app |
| `B:` | Baliza acústica de llegada |
| `I:<texto>` | Indicación de navegación |

---

## PÁGINA 16 — Motor de Decisión (Prioridades)

### Jerarquía de prioridades

El sistema evalúa los eventos en orden de prioridad descendente:

```
PRIORIDAD_CRITICA (3)    ← Emergencia SOS / Caída
    │
    ▼
PRIORIDAD_ALTA (2)       ← Pozo / Batería baja
    │
    ▼
PRIORIDAD_NORMAL (1)     ← Obstáculos frontal/radar
    │
    ▼
PRIORIDAD_BAJA (0)       ← Sin eventos
```

### Flujo de decisión

```
┌─────────────────┐
│ Botón SOS?      │──SI──► EMERGENCIA (todos vibran)
└────────┬────────┘
         │ NO
┌────────▼────────┐
│ Caída MPU?      │──SI──► CAIDA (todos vibran máx)
└────────┬────────┘
         │ NO
┌────────▼────────┐
│ Pozo/desnivel?  │──SI──► Motor centro vibra PWM_POZO
└────────┬────────┘
         │ NO
┌────────▼────────┐
│ Obstáculo       │──SI──► Motor(es) según dirección
│ frontal/radar?  │        e intensidad según distancia
└────────┬────────┘
         │ NO
┌────────▼────────┐
│ Todo normal     │──────► Motores apagados
└─────────────────┘
```

---

## PÁGINA 17 — Estado Global del Sistema

### Patrón Singleton estático

La clase `Estado` almacena **todo el estado del sistema** en 4 estructuras estáticas:

| Estructura | Contenido | Quién escribe | Quién lee |
|-----------|-----------|---------------|-----------|
| `sensores` | Distancias, aceleración, botón | Sensores, Radar, Movimiento, Emergencia | Decision |
| `salida` | PWM de motores, audio | Decision | Salida, Audio |
| `sistema` | Estado, evento, batería, GPS, BT | Todos los módulos | Todos |
| `app` | Navegación, GPS app, micrófono | Bluetooth (comandos) | Varios |

### Diagrama de flujo de datos

```
Sensores ──────┐
Radar ─────────┤
Movimiento ────┼──► Estado::sensores ──► Decision ──► Estado::salida ──► Salida
Emergencia ────┤                                                         Audio
               │
GPS ──────────► Estado::sistema ◄──── Batería
Bluetooth ────► Estado::app     ◄──── Diagnóstico
```

---

## PÁGINA 18 — Notas de Hardware Importantes

### Divisores de tensión (5V → 3.3V)

Los HC-SR04 operan a 5V y sus pines ECHO pueden dañar el ESP32 (3.3V máx).
Se requiere un divisor resistivo en cada ECHO:

```
    ECHO HC-SR04 (5V) ─── R1(1kΩ) ─── GPIO ESP32 ─── R2(2kΩ) ─── GND
                                          │
                                    V = 5V × 2k/(1k+2k) = 3.33V ✓
```

### Drivers MOSFET para motores

Los motores vibradores consumen más corriente de la que el ESP32 puede
entregar directamente (máx 12mA por GPIO). Se usan MOSFETs canal N:

```
    GPIO ESP32 (3.3V PWM) ──┐
                            │
                        ┌───▼───┐
                        │ Gate  │
              5V ──Motor──Drain │  MOSFET Canal N
                        │ Source│  (Logic Level 3.3V)
                        └───┬───┘
                            │
                           GND
```

### Especificaciones de consumo estimado

| Componente | Consumo típico |
|-----------|:--------------:|
| ESP32 (activo WiFi+BT) | ~160 mA |
| 3x HC-SR04 | ~45 mA |
| MPU-6050 | ~3.5 mA |
| Servo SG90 | ~200 mA (pico) |
| 3x Motores vibradores | ~300 mA |
| MAX98357A + parlante | ~200 mA |
| GPS NEO-6M | ~45 mA |
| **Total estimado** | **~950 mA** |

Con 2x 18650 de 2600mAh: **~5.5 horas** de autonomía estimada.

---

## PÁGINA 19 — Cómo Compilar y Cargar el Firmware

### Requisitos previos

1. **Arduino IDE** 2.x instalado
2. **ESP32 Board Package** instalado:
   - Archivo → Preferencias → URLs adicionales:
   - `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Herramientas → Gestor de tarjetas → buscar "ESP32" → Instalar

3. **Librerías requeridas** (Gestor de Librerías):
   - `ESP32Servo` por Kevin Harrington
   - `MPU6050_light` por rfetick
   - `BluetoothSerial` (incluida con ESP32 core)

### Configuración de placa

| Parámetro | Valor |
|-----------|-------|
| Placa | ESP32 Dev Module |
| CPU Frequency | 240 MHz |
| Flash Size | 4MB |
| Upload Speed | 921600 |
| Partition Scheme | Default 4MB with spiffs |

### Pasos para compilar

1. Abrir `Centinela_v2.ino` en Arduino IDE
2. Seleccionar la placa ESP32 Dev Module
3. Seleccionar el puerto COM correcto
4. Click en "Verificar" (✓) para compilar
5. Click en "Subir" (→) para cargar al ESP32
6. Abrir Monitor Serial a 115200 baud para ver diagnóstico

### Verificación exitosa

Al encender, el monitor serial debe mostrar:

```
======================================
CENTINELA v2.0 — SISTEMA INTEGRADO
ESP32 + I2S Audio + GPS + Microfono + TRRS
Iniciando subsistemas de hardware...
======================================
MPU-6050 OK
Modulo GPS NEO-6M iniciado en Serial2
Audio I2S iniciado (MAX98357A + TRRS Switch)
BT SPP iniciado como: Centinela-Baston
======================================
Centinela v2.0 Listo y Operativo.
Esperando vinculacion automatica de la app...
======================================
```

---

## PÁGINA 20 — Posibles Mejoras Futuras

### Mejoras de hardware

| Mejora | Descripción | Beneficio |
|--------|-------------|-----------|
| ESP32-S3 | Procesador más nuevo con USB nativo | Mejor rendimiento y compatibilidad |
| Sensor TOF (VL53L0X) | Reemplazar HC-SR04 por láser | Mayor precisión y menor consumo |
| Micrófono I2S (INMP441) | Reemplazar ADC analógico | Mejor calidad de audio |
| BLE 5.0 | En lugar de Bluetooth Clásico | Menor consumo de batería |
| Carga USB-C | Módulo TP4056 integrado | Carga sin desmontar baterías |
| PCB diseñado | Reemplazar protoboard | Menor tamaño, mayor confiabilidad |

### Mejoras de software

| Mejora | Descripción |
|--------|-------------|
| FreeRTOS | Usar tareas del RTOS nativo del ESP32 |
| OTA Updates | Actualización del firmware por WiFi |
| Machine Learning | Clasificar tipos de obstáculo con TinyML |
| Modo ahorro | Deep sleep cuando no se detecta movimiento |
| Mapeo de rutas | Guardar recorridos frecuentes en SPIFFS |
| Alertas por voz | TTS local en el ESP32 |

### Impacto social esperado

> "Centinela no busca reemplazar al bastón blanco, sino potenciarlo con
> tecnología accesible para que cada persona con discapacidad visual pueda
> caminar con mayor seguridad, autonomía y confianza."

---

*Documento generado automáticamente para exposición académica.*  
*Todos los archivos fuente están documentados con comentarios en español.*  
*Backup de la versión anterior disponible en: `Centinela_v2_backup/`*
