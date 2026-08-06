/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Decision.h
 * Descripción:
 * Cerebro de decisiones del sistema.
 **************************************************************************/

#ifndef DECISION_H
#define DECISION_H


#include <Arduino.h>

#include "Configuracion.h"
#include "Tipos.h"



class Decision
{

public:

    static void iniciar();

    static void actualizar();



private:

    static int calcularIntensidad(int distancia);

    static void analizarCaida();

    static void analizarSuelo();

    static void analizarObstaculos();



};



#endif