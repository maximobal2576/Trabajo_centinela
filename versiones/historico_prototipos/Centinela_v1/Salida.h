/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Salida.h
 * Descripción:
 * Control de salidas físicas del sistema.
 **************************************************************************/

#ifndef SALIDA_H
#define SALIDA_H


#include <Arduino.h>

#include "Configuracion.h"



class Salida
{

public:

    static void iniciar();

    static void actualizar();



private:

    static void escribirMotor(int pin, int valor);

    static void actualizarBuzzer();
};



#endif