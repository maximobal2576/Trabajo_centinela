/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Scheduler.cpp
 * Descripcion:
 *   Planificador cooperativo sin delay(). Gestiona todos los componentes
 *   del sistema v2 en tiempos precisos.
 **************************************************************************/
#include "Scheduler.h"
#include "Configuracion.h"

#include "Sensores.h"
#include "Radar.h"
#include "Movimiento.h"
#include "Decision.h"
#include "Salida.h"
#include "Bluetooth.h"
#include "Audio.h"
#include "GPS.h"
#include "Emergencia.h"
#include "Microfono.h"
#include "Bateria.h"
#include "Diagnostico.h"

unsigned long Scheduler::tiempoSensores   = 0;
unsigned long Scheduler::tiempoRadar      = 0;
unsigned long Scheduler::tiempoMovimiento = 0;
unsigned long Scheduler::tiempoDecision   = 0;
unsigned long Scheduler::tiempoSalida     = 0;
unsigned long Scheduler::tiempoBluetooth  = 0;
unsigned long Scheduler::tiempoAudio      = 0;
unsigned long Scheduler::tiempoGPS        = 0;
unsigned long Scheduler::tiempoEmergencia = 0;
unsigned long Scheduler::tiempoMicrofono  = 0;
unsigned long Scheduler::tiempoBateria    = 0;

void Scheduler::iniciar()
{
    unsigned long ahora = millis();
    tiempoSensores   = ahora;
    tiempoRadar      = ahora;
    tiempoMovimiento = ahora;
    tiempoDecision   = ahora;
    tiempoSalida     = ahora;
    tiempoBluetooth  = ahora;
    tiempoAudio      = ahora;
    tiempoGPS        = ahora;
    tiempoEmergencia = ahora;
    tiempoMicrofono  = ahora;
    tiempoBateria    = ahora;
}

void Scheduler::actualizar()
{
    unsigned long ahora = millis();

    // 1. Pulsador SOS de Emergencia (prioridad y muestreo rapido)
    if (ahora - tiempoEmergencia >= TIEMPO_EMERGENCIA)
    {
        tiempoEmergencia = ahora;
        Emergencia::actualizar();
    }

    // 2. Microfono (muestreo y streaming continuo de voz a la app)
    Microfono::actualizar();

    // 3. Sensores ultrasonicos HC-SR04
    if (ahora - tiempoSensores >= TIEMPO_SENSORES)
    {
        tiempoSensores = ahora;
        Sensores::actualizar();
    }

    // 4. Radar Servomotor
    if (ahora - tiempoRadar >= TIEMPO_RADAR)
    {
        tiempoRadar = ahora;
        Radar::actualizar();
    }

    // 5. Movimiento e Inclinacion MPU-6050
    if (ahora - tiempoMovimiento >= TIEMPO_MOVIMIENTO)
    {
        tiempoMovimiento = ahora;
        Movimiento::actualizar();
    }

    // 6. GPS NEO-6M
    if (ahora - tiempoGPS >= TIEMPO_GPS)
    {
        tiempoGPS = ahora;
        GPS::actualizar();
    }

    // 7. Telemetria de Bateria 2x 18650
    if (ahora - tiempoBateria >= TIEMPO_BATERIA)
    {
        tiempoBateria = ahora;
        Bateria::actualizar();
    }

    // 8. Logica de Decision
    if (ahora - tiempoDecision >= TIEMPO_DECISION)
    {
        tiempoDecision = ahora;
        Decision::actualizar();
    }

    // 9. Salida a los 3 Motores Vibradores
    if (ahora - tiempoSalida >= TIEMPO_SALIDA)
    {
        tiempoSalida = ahora;
        Salida::actualizar();
    }

    // 10. Bluetooth SPP
    if (ahora - tiempoBluetooth >= TIEMPO_BLUETOOTH)
    {
        tiempoBluetooth = ahora;
        Bluetooth::actualizar();
    }

    // 11. Audio I2S (MAX98357A + Deteccion de Auricular TRRS)
    if (ahora - tiempoAudio >= TIEMPO_AUDIO)
    {
        tiempoAudio = ahora;
        Audio::actualizar();
    }

    Diagnostico::actualizar();
}
