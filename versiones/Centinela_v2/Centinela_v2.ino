/**************************************************************************
 * @file    Centinela_v2.ino
 * @brief   Archivo principal del proyecto Centinela v2.0 (VERSIÓN DEFINITIVA)
 * @version 2.0
 *
 * @details
 *   Bastón inteligente asistivo para personas con discapacidad visual.
 *   Hardware: ESP32-WROOM-32 (240 MHz, Dual Core, WiFi + Bluetooth)
 *   Plataforma: Arduino core para ESP32 (compatible con v2.x y v3.x)
 *
 * 15 COMPONENTES FÍSICOS INTEGRADOS Y VALIDADOS:
 *   1.  ESP32-WROOM-32          — Microcontrolador central 240MHz
 *   2.  HC-SR04 Frontal         — Sensor ultrasónico torso/cabeza (GPIO 23/34)
 *   3.  HC-SR04 Suelo           — Sensor ultrasónico pozos/escalones (GPIO 19/35)
 *   4.  HC-SR04 Radar           — Sensor ultrasónico rotativo (GPIO 2/36)
 *   5.  Servomotor SG90         — Barrido angular 45° a 135° (GPIO 13)
 *   6.  MPU-6050 (I2C)          — Acelerómetro/Giroscopio detección caídas (GPIO 21/22)
 *   7.  3x Motores vibradores   — Feedback háptico Izq / Ctr / Der (GPIO 4, 5, 18)
 *   8.  3x Drivers MOSFET N-Ch  — Conmutación lógica 3.3V para motores
 *   9.  MAX98357A (I2S)         — DAC mono 3W audio digital (GPIO 25, 26, 27, 32)
 *   10. Parlante 4Ω / 3W        — Emisión de audio y sirena de auxilio
 *   11. Jack 3.5mm TRRS         — Detección y conmutación auriculares (GPIO 14)
 *   12. Micrófono electret      — Captura de voz ambiental VAD (GPIO 39)
 *   13. GPS NEO-6M (UART2)      — Coordenadas satelitales NMEA (GPIO 16/17)
 *   14. Pulsador SOS físico     — Botón de pánico táctil con antirrebote (GPIO 15)
 *   15. Sistema 2x 18650        — Alimentación 7.4V/8.4V + Step-Down + ADC (GPIO 33)
 *
 * LIBRERÍAS REQUERIDAS:
 *   - ESP32Servo      (por Kevin Harrington)
 *   - MPU6050_light   (por rfetick)
 *   - BluetoothSerial (incluida en el core de ESP32)
 *
 * CONFIGURACIÓN EN ARDUINO IDE:
 *   - Placa: "ESP32 Dev Module"
 *   - CPU Frequency: 240 MHz
 *   - Flash Size: 4MB
 *   - Upload Speed: 921600
 **************************************************************************/

// =====================================================
// INCLUSIÓN DE MÓDULOS DEL SISTEMA
// =====================================================

// Configuración y estado global
#include "Configuracion.h"
#include "Tipos.h"
#include "Estado.h"
#include "Scheduler.h"

// Percepción sensorial
#include "Sensores.h"
#include "Radar.h"
#include "Movimiento.h"

// Procesamiento y actuadores
#include "Decision.h"
#include "Salida.h"

// Comunicaciones y periféricos
#include "GPS.h"
#include "Emergencia.h"
#include "Microfono.h"
#include "Bluetooth.h"
#include "Audio.h"

// Monitoreo y diagnóstico
#include "Bateria.h"
#include "Diagnostico.h"

// =====================================================
// SETUP — Inicialización del sistema
// =====================================================

void setup() {
    Serial.begin(115200);

    Serial.println(F("========================================"));
    Serial.println(F("  CENTINELA v2.0 — SISTEMA DEFINITIVO   "));
    Serial.println(F("  Bastón Inteligente de Navegación Segura"));
    Serial.println(F("  Iniciando subsistemas...              "));
    Serial.println(F("========================================"));

    // 1. Estado global del sistema (primer módulo obligatorio)
    Estado::iniciar();

    // 2. Sensores de distancia ultrasónicos y servomotor
    Sensores::iniciar();
    Radar::iniciar();

    // 3. Sensor inercial MPU-6050 (calibración en reposo)
    Movimiento::iniciar();

    // 4. Lógica de decisión y motores hápticos
    Decision::iniciar();
    Salida::iniciar();

    // 5. GPS NEO-6M por Hardware Serial2
    GPS::iniciar();

    // 6. Botón físico de emergencia SOS
    Emergencia::iniciar();

    // 7. Micrófono analógico
    Microfono::iniciar();

    // 8. Bluetooth SPP dual
    Bluetooth::iniciar();

    // 9. Audio I2S MAX98357A con conmutador TRRS
    Audio::iniciar();

    // 10. Monitoreo de batería y diagnóstico por consola
    Bateria::iniciar();
    Diagnostico::iniciar();

    // 11. Planificador cooperativo en tiempo real (último módulo)
    Scheduler::iniciar();

    Serial.println(F("========================================"));
    Serial.println(F("  Centinela v2.0 — 100% Operativo       "));
    Serial.println(F("  Listo para asistir al usuario         "));
    Serial.println(F("========================================"));
}

// =====================================================
// LOOP — Bucle principal no bloqueante
// =====================================================

void loop() {
    Scheduler::actualizar();
}
