#ifndef CONFIGURACION_H
#define CONFIGURACION_H

// =============================================================================
// CENTINELA v2.0 — CONFIGURACION GENERAL DE HARDWARE
// Plataforma: ESP32-WROOM-32 (38 pines DevKit)
// =============================================================================
// LISTA COMPLETA DE COMPONENTES SOPORTADOS:
// 1. ESP32-WROOM-32 (Microcontrolador principal)
// 2. 3x Sensores ultrasonicos HC-SR04 (Frontal, Suelo/Pozo, Radar Servo)
// 3. MPU-6050 (Acelerometro y giroscopio I2C para caidas)
// 4. Servomotor SG90 (Barrido angular del radar)
// 5. 3x Motores vibradores de alta potencia (Feedback haptico Izq/Ctr/Der)
// 6. Drivers MOSFET canal N (Conmutacion logica 3.3V a 5V para motores)
// 7. DAC de audio I2S (MAX98357A mono 3W)
// 8. Parlante 4 Ohm / 3 W
// 9. TRRS Breakout (Jack 3.5mm para auricular y microfono)
// 10. Deteccion de auricular (Mute automatico del altavoz al conectar)
// 11. Microfono (TRRS Sleeve o modulo analogico)
// 12. Modulo GPS NEO-6M (Comunicacion UART Serial2)
// 13. Pulsador de emergencia SOS (Con pull-up para alerta instantanea)
// 14. Portabaterias 2x 18650 + 2x Baterias 18650 (7.4V nominal / 8.4V max)
// 15. Regulador de voltaje Step-Down (Buck 5V / 3.3V)
// 16. Fusible de proteccion + Interruptor general ON/OFF
// 17. Divisores resistivos de seguridad para Echos y ADC bateria
// =============================================================================

// =====================================================
// 1. MPU-6050 (I2C)
// =====================================================
#define MPU_SDA                21
#define MPU_SCL                22
#define MPU_UMBRAL_CAIDA_LIBRE 0.45f
#define MPU_UMBRAL_GOLPE       1.75f

// =====================================================
// 2. MODULO GPS NEO-6M (Serial2 por hardware)
// =====================================================
#define GPS_RX_PIN             16    // ESP32 RX2 <- Conectar al TX del NEO-6M
#define GPS_TX_PIN             17    // ESP32 TX2 -> Conectar al RX del NEO-6M
#define GPS_BAUDRATE           9600

// =====================================================
// 3. AUDIO I2S (MAX98357A) Y TRRS BREAKOUT (AURICULARES)
// =====================================================
// Pines del bus digital I2S nativo
#define I2S_BCK_PIN            26    // Bit Clock
#define I2S_WS_PIN             25    // Word Select (LRC)
#define I2S_DATA_PIN           27    // Serial Data (DIN)

// Control de Altavoz (SD_MODE del MAX98357A):
//   HIGH (3.3V) = Amplificador habilitado -> Parlante 4 Ohm / 3 W activo
//   LOW  (0V)   = Shutdown total (<0.6uA)  -> Parlante 100% SILENCIADO
#define I2S_SD_PIN             32

// Deteccion de Auricular en TRRS Breakout:
// Pin de deteccion fisica (Jack Switch):
//   Sin auricular insertado : HIGH (Pull-up interno activo)
//   Con auricular insertado : LOW  (Contacto cierra a GND al enchufar jack)
#define PIN_JACK_DETECT        33
#define NIVEL_JACK_CONECTADO   LOW

// Parametros de transmision de audio PCM
#define AUDIO_SAMPLE_RATE      16000
#define AUDIO_BITS_PER_SAMPLE  16
#define AUDIO_CHANNELS         1
#define I2S_PORT               I2S_NUM_0
#define I2S_DMA_BUF_COUNT      8
#define I2S_DMA_BUF_LEN        512
#define AUDIO_BUF_SIZE         2048
#define AUDIO_STREAM_TIMEOUT_MS 400

// =====================================================
// 4. 3x MOTORES VIBRADORES (LEDC PWM via MOSFETs)
// =====================================================
#define MOTOR_IZQUIERDO        4
#define MOTOR_CENTRO           5
#define MOTOR_DERECHO          18

#define LEDC_CH_IZQ            0
#define LEDC_CH_CTR            1
#define LEDC_CH_DER            2
#define LEDC_FREQ_HZ           5000
#define LEDC_BITS              8

#define PWM_SUAVE              80
#define PWM_MEDIO              150
#define PWM_FUERTE             210
#define PWM_MAXIMO             255
#define PWM_POZO               130

// =====================================================
// 5. SERVO RADAR (Barrido angular)
// =====================================================
#define PIN_SERVO              13
#define RADAR_IZQUIERDA        45
#define RADAR_CENTRO           90
#define RADAR_DERECHA          135
#define RADAR_PASO             5
#define RADAR_SETTLE_MS        0
#define RADAR_MEDICIONES       1

// =====================================================
// 6. 3x SENSORES ULTRASONICOS HC-SR04
// =====================================================
// NOTA IMPORTANTE: Los pines ECHO de 5V deben conectarse a traves
// de un divisor de tension (R1=1k, R2=2k) para entregar 3.3V seguros al ESP32.
#define TRIG_FRONTAL           23
#define ECHO_FRONTAL           35    // GPIO 35 (Input-only, seguro con divisor)

#define TRIG_SUELO             19
#define ECHO_SUELO             36    // GPIO 36 / VP (Input-only con divisor)

#define TRIG_RADAR             2
#define ECHO_RADAR             39    // GPIO 39 / VN (Input-only con divisor)

#define DISTANCIA_ALERTA_MAX   80
#define DISTANCIA_PELIGRO      20

// =====================================================
// 7. PULSADOR DE EMERGENCIA (SOS)
// =====================================================
// Conectar pulsador entre GPIO 15 y GND (usa INPUT_PULLUP interno)
#define PIN_BOTON_SOS          15
#define NIVEL_SOS_PRESIONADO   LOW
#define SOS_DEBOUNCE_MS        50

// =====================================================
// 8. BATERIA 2x 18650 (Telemetria ADC 12-bit)
// =====================================================
// Baterias 2S: Maximo 8.4V, Nominal 7.4V, Vacia 6.0V
// Divisor resistivo recomendado: R1=100k (a VBat), R2=33k (a GND)
// A 8.4V: V_pin = 8.4V * (33/133) = 2.08V (seguro para el ADC de 3.3V)
#define PIN_BATERIA            34    // GPIO 34 (ADC1_CH6)
#define BATERIA_MUESTRAS       8
#define BATERIA_UMBRAL_BAJA    15
#define BATERIA_UMBRAL_RECUPERACION 18
#define ADC_LECTURA_VACIA      1910  // ~6.0V
#define ADC_LECTURA_LLENA      2670  // ~8.4V

// =====================================================
// 9. BLUETOOTH SPP
// =====================================================
#define BT_NOMBRE_DISPOSITIVO  "Centinela-Baston"
#define BT_PIN_PAREO           "1234"

// =====================================================
// 10. TIEMPOS DEL SCHEDULER (milisegundos)
// =====================================================
#define TIEMPO_SENSORES        40
#define TIEMPO_RADAR           60
#define TIEMPO_MOVIMIENTO      20
#define TIEMPO_DECISION        20
#define TIEMPO_SALIDA          20
#define TIEMPO_BLUETOOTH       150
#define TIEMPO_AUDIO           20
#define TIEMPO_GPS             200
#define TIEMPO_EMERGENCIA      50
#define TIEMPO_BATERIA         4000
#define TIEMPO_DIAGNOSTICO     1000
#define MODO_DIAGNOSTICO       true

#endif // CONFIGURACION_H
