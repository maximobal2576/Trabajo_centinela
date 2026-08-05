/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Sensores.h
 * Descripción:
 * Gestión de sensores ultrasónicos.
 **************************************************************************/

#ifndef SENSORES_H
#define SENSORES_H


#include <Arduino.h>

#include "Configuracion.h"


class Sensores
{

public:

    static void iniciar();

    static void actualizar();



private:

    static int leerDistancia(int trig, int echo);

};



#endif