/**
 * @file    Scheduler.h
 * @brief   Planificador cooperativo en tiempo real basado en millis() — Centinela v2.0
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

class Scheduler {
public:
    static void iniciar();
    static void actualizar();

private:
    Scheduler() = delete;
    static uint32_t _tEmergencia;
    static uint32_t _tMicrofono;
    static uint32_t _tMovimiento;
    static uint32_t _tSensores;
    static uint32_t _tRadar;
    static uint32_t _tDecision;
    static uint32_t _tSalida;
    static uint32_t _tAudio;
    static uint32_t _tGps;
    static uint32_t _tBluetooth;
    static uint32_t _tBateria;
    static uint32_t _tDiagnostico;
};

#endif // SCHEDULER_H
