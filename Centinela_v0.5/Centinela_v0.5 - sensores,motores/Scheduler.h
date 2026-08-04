/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Scheduler.h
 * Descripción:
 * Controlador principal de tiempos del sistema.
 **************************************************************************/

#ifndef SCHEDULER_H
#define SCHEDULER_H


#include <Arduino.h>
#include "Configuracion.h"



class Scheduler
{

public:

    static void iniciar();

    static void actualizar();



private:


    static unsigned long tiempoSensores;

    static unsigned long tiempoRadar;

    static unsigned long tiempoMPU;


    static unsigned long tiempoDecision;

    static unsigned long tiempoSalida;

    static unsigned long tiempoBluetooth;

    static unsigned long tiempoAudio;

    static unsigned long tiempoBateria;


};


#endif