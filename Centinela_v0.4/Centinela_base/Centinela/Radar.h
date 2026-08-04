#ifndef RADAR_H
#define RADAR_H


#include <Arduino.h>


/*
======================================================

                CENTINELA V2

                    RADAR

  Archivo:
  Radar.h


  Funcion:

  Control del servo radar y barrido
  de deteccion de obstaculos.


======================================================
*/



class Radar
{

public:


    static void iniciar();


    static void actualizar();



private:


    enum class EstadoRadarInterno
    {

        MOVER_IZQUIERDA,

        LEER_IZQUIERDA,

        MOVER_CENTRO,

        LEER_CENTRO,

        MOVER_DERECHA,

        LEER_DERECHA

    };



    static EstadoRadarInterno estadoActual;



    static unsigned long tiempoAnterior;



    static void moverServo(
        uint8_t posicion
    );


};



#endif