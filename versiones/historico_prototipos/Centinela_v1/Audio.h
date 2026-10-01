/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Audio.h
 * Descripción:
 * Control del módulo DFPlayer Mini.
 **************************************************************************/

#ifndef AUDIO_H
#define AUDIO_H


#include <Arduino.h>

#include "Configuracion.h"



class Audio
{

public:

    static void iniciar();

    static void actualizar();



private:

    static void reproducir(int id);



};



#endif