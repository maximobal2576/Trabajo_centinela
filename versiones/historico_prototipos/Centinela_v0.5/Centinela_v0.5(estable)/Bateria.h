/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Bateria.h
 * Descripción:
 * Lectura y gestión del nivel de batería.
 **************************************************************************/

#ifndef BATERIA_H
#define BATERIA_H


#include <Arduino.h>

#include "Configuracion.h"



class Bateria
{

public:

    static void iniciar();

    static void actualizar();



private:

    static int calcularPorcentaje(int lectura);



};



#endif