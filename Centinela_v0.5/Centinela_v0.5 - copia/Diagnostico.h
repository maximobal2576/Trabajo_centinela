/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Diagnostico.h
 * Descripción:
 * Herramientas de diagnóstico y pruebas.
 **************************************************************************/

#ifndef DIAGNOSTICO_H
#define DIAGNOSTICO_H


#include <Arduino.h>


class Diagnostico
{

public:

    static void iniciar();

    static void actualizar();



private:

    static void imprimirEstado();



};



#endif