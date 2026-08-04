/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Movimiento.h
 * Descripción:
 * Gestión del sensor MPU6050.
 **************************************************************************/

#ifndef MOVIMIENTO_H
#define MOVIMIENTO_H


#include <Arduino.h>
#include <Wire.h>

#include "Configuracion.h"


class Movimiento
{

public:

    static void iniciar();

    static void actualizar();



private:

    static float aceleracionAnterior;

    static bool detectarCaida();


};



#endif