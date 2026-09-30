/**
 * @file    Configuracion.h
 * @brief   Archivo central de configuración — Centinela v2.0 (VERSIÓN DEFINITIVA)
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │  MAPA DE PINES ESP32-WROOM-32 (38 pines)                               │
 * │                                                                         │
 * │  IMPORTANTE — Restricciones del ESP32:                                  │
 * │  • GPIO 34, 35, 36, 39 → solo INPUT (sin pullup interno)               │
 * │  • GPIO 6–11           → flash SPI interno, NO USAR                    │
 * │  • GPIO 1, 3           → UART0 (Serial USB debug), NO USAR             │
 * │  • ADC2 (GPIO 0,2,4,12–15,25–27) → NO funciona con Bluetooth activo   │
 * │  • ADC1 (GPIO 32–39)  → SIEMPRE disponible, incluso con BT activo     │
 * │                                                                         │
 * │  GPIO 2  → TRIG_RADAR      (OUTPUT, pull-down externo recomendado)     │
 * │  GPIO 4  → MOTOR_IZQUIERDO (OUTPUT PWM LEDC)                          │
 * │  GPIO 5  → MOTOR_CENTRO    (OUTPUT PWM LEDC)                          │
 * │  GPIO 13 → PIN_SERVO       (OUTPUT servo SG90)                         │
 * │  GPIO 14 → PIN_JACK_DETECT (INPUT_PULLUP, detección auriculares TRRS) │
 * │  GPIO 15 → PIN_BOTON_SOS   (INPUT_PULLUP, botón pánico)               │
 * │  GPIO 16 → GPS_RX          (UART2 recepción NEO-6M)                   │
 * │  GPIO 17 → GPS_TX          (UART2 transmisión, no conectar si no usas)│
 * │  GPIO 18 → MOTOR_DERECHO   (OUTPUT PWM LEDC)                          │
 * │  GPIO 19 → TRIG_SUELO      (OUTPUT)                                   │
 * │  GPIO 21 → MPU_SDA         (I2C datos)                                │
 * │  GPIO 22 → MPU_SCL         (I2C reloj)                                │
 * │  GPIO 23 → TRIG_FRONTAL    (OUTPUT)                                   │
 * │  GPIO 25 → I2S_WS_PIN      (I2S Word Select / LRCLK → MAX98357A)     │
 * │  GPIO 26 → I2S_BCK_PIN     (I2S Bit Clock → MAX98357A)               │
 * │  GPIO 27 → I2S_DATA_PIN    (I2S Data Out → MAX98357A)                │
 * │  GPIO 32 → I2S_SD_PIN      (OUTPUT, shutdown amplificador MAX98357A) │
 * │  GPIO 33 → PIN_BATERIA     (ADC1_CH5, divisor resistivo de tensión)  │
 * │  GPIO 34 → ECHO_FRONTAL    (INPUT-ONLY, eco ultrasónico frontal)     │
 * │  GPIO 35 → ECHO_SUELO      (INPUT-ONLY, eco ultrasónico suelo)       │
 * │  GPIO 36 → ECHO_RADAR      (INPUT-ONLY, eco ultrasónico servo/radar) │
 * │  GPIO 39 → PIN_MICROFONO   (ADC1_CH3, micrófono analógico electret)  │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * LIBRERÍAS NECESARIAS (Gestor de Librerías de Arduino IDE):
 *   • ESP32Servo      → por Kevin Harrington (control del SG90)
 *   • MPU6050_light   → por rfetick (IMU acelerómetro/giroscopio)
 *   • BluetoothSerial → incluida con arduino-esp32 (BT SPP clásico)
 *
 * CONFIGURACIÓN ARDUINO IDE:
 *   Placa        : "ESP32 Dev Module"
 *   CPU Frequency: 240 MHz
 *   Flash Size   : 4MB
 *   Upload Speed : 921600
 *   PSRAM        : Disabled (WROOM) / Enabled (WROVER)
 *
 * VERSIÓN CORE RECOMENDADA:
 *   arduino-esp32 v2.0.x — probado y estable con toda la API usada aquí.
 *   arduino-esp32 v3.x   — compatible gracias a los bloques #if ESP_ARDUINO_VERSION_MAJOR
 *                          en Salida.cpp y Audio.cpp
 */

#ifndef CONFIGURACION_H
#define CONFIGURACION_H

// ============================================================================
// IMU — ACELERÓMETRO / GIROSCOPIO (MPU-6050)
// ============================================================================
#define MPU_SDA                 21    ///< Pin I2C SDA (datos)
#define MPU_SCL                 22    ///< Pin I2C SCL (reloj)
#define MPU_UMBRAL_CAIDA_LIBRE  0.45f ///< Umbral G para caída libre (≈ 0 G = levitando)
#define MPU_UMBRAL_GOLPE        1.75f ///< Umbral G para golpe/impacto brusco (> 1 G normal)

// ============================================================================
// GPS — MÓDULO NEO-6M (UART2)
// ============================================================================
#define GPS_RX_PIN              16    ///< ESP32 RX ← TX del NEO-6M
#define GPS_TX_PIN              17    ///< ESP32 TX → RX del NEO-6M (puede quedar sin conectar)
#define GPS_BAUDRATE            9600  ///< Velocidad estándar del NEO-6M de fábrica

// ============================================================================
// AUDIO I2S — AMPLIFICADOR MAX98357A (ALTAVOZ 3W) + JACK TRRS 3.5mm
// ============================================================================
#define I2S_BCK_PIN             26    ///< Bit Clock (BCLK → MAX98357A pin BCLK)
#define I2S_WS_PIN              25    ///< Word Select / LRCLK (→ MAX98357A pin LRC)
#define I2S_DATA_PIN            27    ///< Datos I2S (→ MAX98357A pin DIN)
#define I2S_SD_PIN              32    ///< Shutdown del amplificador: HIGH=ON, LOW=OFF (silencio)
#define PIN_JACK_DETECT         14    ///< Detección de auriculares TRRS: LOW=conectado
#define NIVEL_JACK_CONECTADO    LOW   ///< Nivel lógico que indica auriculares presentes

// Parámetros del stream de audio PCM
#define AUDIO_SAMPLE_RATE       16000 ///< Frecuencia de muestreo: 16 kHz (voz)
#define AUDIO_BITS_PER_SAMPLE   16    ///< 16 bits por muestra (PCM estándar)
#define AUDIO_CHANNELS          1     ///< 1 = Mono (ahorra ancho de banda BT)
#define I2S_PORT                I2S_NUM_0  ///< Puerto I2S del ESP32 a utilizar
#define I2S_DMA_BUF_COUNT       8     ///< Cantidad de buffers DMA (más = mayor latencia pero más estable)
#define I2S_DMA_BUF_LEN        512   ///< Tamaño de cada buffer DMA en frames
#define AUDIO_BUF_SIZE          2048  ///< Tamaño del buffer circular de audio (bytes)
#define AUDIO_STREAM_TIMEOUT_MS 400   ///< Tiempo sin datos para considerar el stream terminado

// ============================================================================
// MOTORES DE VIBRACIÓN HÁPTICA — LEDC PWM (3x motor ERM con MOSFET canal N)
// ============================================================================
// Conexión recomendada: GPIO → Gate MOSFET → Motor entre Drain y 5V/VCC
//   Motor izquierdo: avisa obstáculos a la izquierda
//   Motor central  : avisa obstáculos frontales / pozos / modo caída
//   Motor derecho  : avisa obstáculos a la derecha
#define MOTOR_IZQUIERDO         4     ///< GPIO → Gate MOSFET motor izquierdo
#define MOTOR_CENTRO            5     ///< GPIO → Gate MOSFET motor central
#define MOTOR_DERECHO           18    ///< GPIO → Gate MOSFET motor derecho

// Canales LEDC (solo necesarios para arduino-esp32 v2.x)
#define LEDC_CH_IZQ             0     ///< Canal LEDC para motor izquierdo
#define LEDC_CH_CTR             1     ///< Canal LEDC para motor central
#define LEDC_CH_DER             2     ///< Canal LEDC para motor derecho

// Parámetros del PWM
#define LEDC_FREQ_HZ            5000  ///< Frecuencia PWM: 5 kHz (inaudible, eficiente)
#define LEDC_BITS               8     ///< Resolución: 8 bits → rango 0–255

// Niveles de intensidad de vibración (PWM)
#define PWM_SUAVE               80    ///< Vibración suave: obstáculo lejano (60–80 cm)
#define PWM_MEDIO               150   ///< Vibración media: obstáculo a distancia intermedia
#define PWM_FUERTE              210   ///< Vibración fuerte: obstáculo cercano
#define PWM_MAXIMO              255   ///< Vibración máxima: peligro inminente / emergencia
#define PWM_POZO                130   ///< Patrón especial para detectar pozos (motor central)

// ============================================================================
// RADAR — SERVO SG90 + SENSOR ULTRASÓNICO ROTATORIO
// ============================================================================
// El sensor HC-SR04 del radar va montado sobre el eje del servo SG90.
// Barre de 45° a 135° en pasos de RADAR_PASO grados cada TIEMPO_RADAR ms.
#define PIN_SERVO               13    ///< GPIO → señal PWM del servo SG90
#define RADAR_IZQUIERDA         45    ///< Ángulo límite izquierdo del barrido (°)
#define RADAR_CENTRO            90    ///< Ángulo central del barrido (°)
#define RADAR_DERECHA           135   ///< Ángulo límite derecho del barrido (°)
#define RADAR_PASO              5     ///< Incremento angular por ciclo (°)

// ============================================================================
// SENSORES ULTRASÓNICOS — HC-SR04 (x3)
// ============================================================================
// ⚠ Los pines ECHO (34, 35, 36) son INPUT-ONLY en el ESP32.
//   No se pueden configurar como OUTPUT ni tienen pull-up interno.
//   Usar pull-down de 10 kΩ en cada línea ECHO es recomendable.

// -- Sensor FRONTAL: detecta obstáculos a la altura del torso/cabeza
#define TRIG_FRONTAL            23    ///< Trigger frontal (OUTPUT, pulso 10 µs)
#define ECHO_FRONTAL            34    ///< Echo frontal    (INPUT-ONLY, ADC1_CH6)

// -- Sensor SUELO: detecta pozos, bordes de escalera, desniveles bruscos
#define TRIG_SUELO              19    ///< Trigger suelo (OUTPUT)
#define ECHO_SUELO              35    ///< Echo suelo    (INPUT-ONLY, ADC1_CH7)

// -- Sensor RADAR: montado sobre el servo, mide distancias angulares
#define TRIG_RADAR              2     ///< Trigger radar (OUTPUT, pull-down externo recomendado)
#define ECHO_RADAR              36    ///< Echo radar    (INPUT-ONLY, ADC1_CH0)
//    ⚠ GPIO 2 → pull-down externo de 10 kΩ para evitar boot en modo Flash.

// Umbrales de detección
#define DISTANCIA_ALERTA_MAX    80    ///< Distancia (cm) a partir de la cual se alerta al usuario
#define DISTANCIA_PELIGRO       20    ///< Distancia (cm) de peligro inminente → vibración máxima

// ============================================================================
// MICRÓFONO ANALÓGICO ELECTRET
// ============================================================================
// Circuito: micrófono electret con divisor de tensión y condensador de desacople.
// GPIO 39 = ADC1_CH3 = VP (Voltage Plus) en los módulos WROOM.
// ⚠ GPIO 39 es INPUT-ONLY. Usar solo para lectura analógica.
// ⚠ El conflicto original (GPIO 39 compartido con ECHO_RADAR) ha sido resuelto:
//    ECHO_RADAR movido a GPIO 36 → GPIO 39 ahora es EXCLUSIVO del micrófono.
#define PIN_MICROFONO           39    ///< Micrófono electret (ADC1_CH3, INPUT-ONLY)
#define MIC_UMBRAL_VOZ          200   ///< Amplitud mínima ADC para detectar voz (0–2048 centrado)
#define MIC_SILENCIO_TIMEOUT_MS 700   ///< Tiempo de silencio (ms) para cerrar la grabación

// ============================================================================
// BOTÓN SOS / PÁNICO
// ============================================================================
// Conexión: botón entre GPIO 15 y GND (INPUT_PULLUP → LOW cuando se presiona)
#define PIN_BOTON_SOS           15    ///< GPIO del botón de emergencia
#define NIVEL_SOS_PRESIONADO    LOW   ///< Nivel lógico al presionar (activo LOW)
#define SOS_DEBOUNCE_MS         50    ///< Tiempo de debounce en ms (antirrebote por software)

// ============================================================================
// BATERÍA — DIVISOR RESISTIVO + ADC
// ============================================================================
// Circuito: 2 baterías 18650 en serie (7.4V nom. / 8.4V cargada / 6.0V mínima).
// Divisor de voltaje: R1=100kΩ, R2=47kΩ → Vout = Vin × 47/(100+47) ≈ Vin × 0.32
// Esto mapea 8.4V → 2.69V y 6.0V → 1.91V (dentro del rango ADC 0–3.3V).
// GPIO 33 = ADC1_CH5 → compatible con Bluetooth activo.
#define PIN_BATERIA             33    ///< GPIO para medición de voltaje (ADC1_CH5)
#define BATERIA_MUESTRAS        8     ///< Promedio de N muestras para estabilizar la lectura
#define BATERIA_UMBRAL_BAJA     15    ///< Porcentaje bajo el cual se dispara la alarma
#define BATERIA_UMBRAL_RECUPERACION 18 ///< Porcentaje para cancelar la alarma (histéresis)
#define ADC_LECTURA_VACIA       1910  ///< Valor ADC correspondiente a batería vacía (~6.0V)
#define ADC_LECTURA_LLENA       2670  ///< Valor ADC correspondiente a batería llena (~8.4V)

// ============================================================================
// BLUETOOTH SPP (Serial Port Profile)
// ============================================================================
// El ESP32 actúa como servidor BT SPP. La app móvil se conecta a este nombre.
// El PIN de 4 dígitos es el código de emparejamiento.
#define BT_NOMBRE_DISPOSITIVO   "Centinela-Baston" ///< Nombre visible en la búsqueda BT
#define BT_PIN_PAREO            "1234"             ///< PIN de emparejamiento (cambiar en producción)

// ============================================================================
// TIEMPOS DEL SCHEDULER (milisegundos)
// ============================================================================
// El Scheduler es cooperativo (no usa delay ni interrupciones para las tareas).
// Cada tarea se ejecuta cuando su período ha expirado. Si una tarea tarda más
// de su período, el sistema sigue funcionando pero con menor frecuencia.
//
// Prioridad de diseño: EMERGENCIA > MOVIMIENTO > SENSORES > DECISION > SALIDA
#define TIEMPO_EMERGENCIA       50    ///< ms — botón SOS (prioridad máxima)
#define TIEMPO_MOVIMIENTO       20    ///< ms — IMU MPU6050 (detección de caídas en tiempo real)
#define TIEMPO_SENSORES         40    ///< ms — HC-SR04 frontal y suelo
#define TIEMPO_RADAR            60    ///< ms — HC-SR04 + servo (una posición por ciclo)
#define TIEMPO_DECISION         20    ///< ms — lógica de alerta (basada en lecturas recientes)
#define TIEMPO_SALIDA           20    ///< ms — actualización PWM de motores vibradores
#define TIEMPO_AUDIO            20    ///< ms — transferencia de muestras al I2S DMA
#define TIEMPO_BLUETOOTH        150   ///< ms — envío de telemetría a la app móvil
#define TIEMPO_GPS              200   ///< ms — parseo de sentencias NMEA del NEO-6M
#define TIEMPO_MICROFONO        5     ///< ms — muestreo analógico del micrófono (≈ 200 Hz)
#define TIEMPO_BATERIA          4000  ///< ms — lectura ADC de voltaje (no necesita alta frecuencia)
#define TIEMPO_DIAGNOSTICO      1000  ///< ms — reporte de estado por Serial USB

/// @brief Activa/desactiva el reporte de diagnóstico por Serial USB.
/// En producción: poner a false para liberar ciclos de CPU.
#define MODO_DIAGNOSTICO        true

#endif // CONFIGURACION_H
