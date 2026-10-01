#ifndef RADAR_H
#define RADAR_H


#include <Arduino.h>


/*
======================================================

                CENTINELA V2

                  RADAR V2

======================================================
*/


class Radar
{

public:


    static void iniciar();


    static void actualizar();



private:


    enum class PasoRadar
    {

        IR_IZQUIERDA,

        LEER_IZQUIERDA,


        IR_CENTRO,

        LEER_CENTRO,


        IR_DERECHA,

        LEER_DERECHA

    };



    static PasoRadar pasoActual;



    static unsigned long tiempoAnterior;



    static void mover(
        uint8_t posicion
    );



};




#endif