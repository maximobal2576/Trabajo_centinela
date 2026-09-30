/**
 * @file    Scheduler.cpp
 * @brief   Implementación del planificador cooperativo no bloqueante — Centinela v2.0
 */

#include "Scheduler.h"
#include "Configuracion.h"

// Inclusión de todos los módulos del sistema
#include "Emergencia.h"
#include "Microfono.h"
#include "Movimiento.h"
#include "Sensores.h"
#include "Radar.h"
#include "Decision.h"
#include "Salida.h"
#include "Audio.h"
#include "GPS.h"
#include "Bluetooth.h"
#include "Bateria.h"
#include "Diagnostico.h"

uint32_t Scheduler::_tEmergencia   = 0;
uint32_t Scheduler::_tMicrofono    = 0;
uint32_t Scheduler::_tMovimiento   = 0;
uint32_t Scheduler::_tSensores     = 0;
uint32_t Scheduler::_tRadar        = 0;
uint32_t Scheduler::_tDecision     = 0;
uint32_t Scheduler::_tSalida       = 0;
uint32_t Scheduler::_tAudio        = 0;
uint32_t Scheduler::_tGps          = 0;
uint32_t Scheduler::_tBluetooth    = 0;
uint32_t Scheduler::_tBateria      = 0;
uint32_t Scheduler::_tDiagnostico  = 0;

void Scheduler::iniciar() {
    uint32_t ahora = millis();
    _tEmergencia   = ahora;
    _tMicrofono    = ahora;
    _tMovimiento   = ahora;
    _tSensores     = ahora;
    _tRadar        = ahora;
    _tDecision     = ahora;
    _tSalida       = ahora;
    _tAudio        = ahora;
    _tGps          = ahora;
    _tBluetooth    = ahora;
    _tBateria      = ahora;
    _tDiagnostico  = ahora;
    Serial.println(F("  [Scheduler] Planificador iniciado"));
}

void Scheduler::actualizar() {
    uint32_t ahora = millis();

    // 1. PRIORIDAD CRÍTICA: Emergencia SOS
    if (ahora - _tEmergencia >= TIEMPO_EMERGENCIA) {
        _tEmergencia = ahora;
        Emergencia::actualizar();
    }

    // 2. Muestreo analógico del Micrófono (frecuencia alta)
    if (ahora - _tMicrofono >= TIEMPO_MICROFONO) {
        _tMicrofono = ahora;
        Microfono::actualizar();
    }

    // 3. IMU / Detección de Caídas
    if (ahora - _tMovimiento >= TIEMPO_MOVIMIENTO) {
        _tMovimiento = ahora;
        Movimiento::actualizar();
    }

    // 4. Sensores ultrasónicos fijos (Frontal / Suelo)
    if (ahora - _tSensores >= TIEMPO_SENSORES) {
        _tSensores = ahora;
        Sensores::actualizar();
    }

    // 5. Radar rotativo (Servo SG90 + HC-SR04)
    if (ahora - _tRadar >= TIEMPO_RADAR) {
        _tRadar = ahora;
        Radar::actualizar();
    }

    // 6. Lógica de Decisión
    if (ahora - _tDecision >= TIEMPO_DECISION) {
        _tDecision = ahora;
        Decision::actualizar();
    }

    // 7. Salida háptica (Motores vibradores)
    if (ahora - _tSalida >= TIEMPO_SALIDA) {
        _tSalida = ahora;
        Salida::actualizar();
    }

    // 8. Audio I2S / DMA
    if (ahora - _tAudio >= TIEMPO_AUDIO) {
        _tAudio = ahora;
        Audio::actualizar();
    }

    // 9. Geolocalización GPS NMEA
    if (ahora - _tGps >= TIEMPO_GPS) {
        _tGps = ahora;
        GPS::actualizar();
    }

    // 10. Telemetría y comandos Bluetooth
    if (ahora - _tBluetooth >= TIEMPO_BLUETOOTH) {
        _tBluetooth = ahora;
        Bluetooth::actualizar();
    }

    // 11. Batería (muestreo lento)
    if (ahora - _tBateria >= TIEMPO_BATERIA) {
        _tBateria = ahora;
        Bateria::actualizar();
    }

    // 12. Diagnóstico por consola USB
    if (ahora - _tDiagnostico >= TIEMPO_DIAGNOSTICO) {
        _tDiagnostico = ahora;
        Diagnostico::actualizar();
    }
}
