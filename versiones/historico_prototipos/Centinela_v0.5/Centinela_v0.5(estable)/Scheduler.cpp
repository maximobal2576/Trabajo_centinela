/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Scheduler.cpp
 **************************************************************************/

#include "Scheduler.h"

#include "Sensores.h"
#include "Radar.h"
#include "Movimiento.h"
#include "Decision.h"
#include "Salida.h"
#include "Bluetooth.h"
#include "Audio.h"
#include "Bateria.h"
#include "Diagnostico.h"



unsigned long Scheduler::tiempoSensores = 0;
unsigned long Scheduler::tiempoRadar = 0;
unsigned long Scheduler::tiempoMovimiento = 0;
unsigned long Scheduler::tiempoDecision = 0;
unsigned long Scheduler::tiempoSalida = 0;
unsigned long Scheduler::tiempoBluetooth = 0;
unsigned long Scheduler::tiempoAudio = 0;
unsigned long Scheduler::tiempoBateria = 0;



void Scheduler::iniciar()
{

    unsigned long ahora = millis();

    tiempoSensores = ahora;
    tiempoRadar = ahora;
    tiempoMovimiento = ahora;
    tiempoDecision = ahora;
    tiempoSalida = ahora;
    tiempoBluetooth = ahora;
    tiempoAudio = ahora;
    tiempoBateria = ahora;

}



