#ifndef CONFIGURACION_H
#define CONFIGURACION_H

// =====================================================
// SENSORES ULTRASONICOS
// =====================================================

// Sensor frontal
#define TRIG_FRONTAL 24
#define ECHO_FRONTAL 25

// Sensor suelo
#define TRIG_SUELO 26
#define ECHO_SUELO 27

// Radar inferior
#define TRIG_RADAR 22
#define ECHO_RADAR 23

// =====================================================
// SERVO RADAR
// =====================================================

#define PIN_SERVO 10

// Posiciones del barrido
#define RADAR_IZQUIERDA 45
#define RADAR_CENTRO 90
#define RADAR_DERECHA 135

// =====================================================
// MOTORES VIBRADORES (PWM)
// =====================================================

#define MOTOR_IZQUIERDO 5
#define MOTOR_CENTRO 6
#define MOTOR_DERECHO 9
#define MOTOR_FRONTAL 11

// =====================================================
// MPU6050 (I2C)
// =====================================================

#define MPU_SDA 20
#define MPU_SCL 21

// =====================================================
// BUZZER DE EMERGENCIA
// =====================================================

#define BUZZER_PIN 12

// =====================================================
// BLUETOOTH HC-05 (UART1 MEGA)
// =====================================================

#define BLUETOOTH_BAUDRATE 9600

// =====================================================
// DFPLAYER MINI (UART2 MEGA)
// =====================================================

#define AUDIO_BAUDRATE 9600

// =====================================================
// DISTANCIAS DE ALERTA (centímetros)
// =====================================================

#define DISTANCIA_ALERTA_MAX 80
#define DISTANCIA_PELIGRO 20

// =====================================================
// INTENSIDAD PWM
// =====================================================

#define PWM_SUAVE 80
#define PWM_MEDIO 150
#define PWM_FUERTE 210
#define PWM_MAXIMO 255

// Pozo / desnivel
#define PWM_POZO 120

// =====================================================
// TIEMPOS DEL SCHEDULER (milisegundos)
// =====================================================

#define TIEMPO_SENSORES 40
#define TIEMPO_RADAR 60
#define TIEMPO_MOVIMIENTO 20
#define TIEMPO_DECISION 20
#define TIEMPO_SALIDA 20
#define TIEMPO_BLUETOOTH 150
#define TIEMPO_AUDIO 100
#define AUDIO_TIMEOUT_MS 2500
#define TIEMPO_BATERIA 5000

// =====================================================
// BATERIA
// =====================================================

#define PIN_BATERIA A0
#define BATERIA_MUESTRAS 6
#define BATERIA_UMBRAL_BAJA 15
#define BATERIA_UMBRAL_RECUPERACION 18

// =====================================================
// DIAGNOSTICO
// =====================================================

#define MODO_DIAGNOSTICO true
#define TIEMPO_DIAGNOSTICO 1000

// =====================================================
// RADAR
// =====================================================

#define RADAR_PASO 5
#define RADAR_SETTLE_MS 0
#define RADAR_MEDICIONES 1

#endif
