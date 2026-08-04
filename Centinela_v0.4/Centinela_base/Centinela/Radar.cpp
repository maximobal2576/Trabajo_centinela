#ifndef RADAR_CPP
#define RADAR_CPP


#include <Servo.h>


#include "Radar.h"
#include "Configuracion.h"
#include "Estado.h"
#include "Sensores.h"



/*
======================================================

                CENTINELA V2

                    RADAR

  Archivo:
  Radar.cpp


======================================================
*/



Servo servoRadar;



Radar::EstadoRadarInterno Radar::estadoActual =
Radar::EstadoRadarInterno::MOVER_IZQUIERDA;



unsigned long Radar::tiempoAnterior = 0;



void Radar::iniciar()
{

    servoRadar.attach(
        PIN_SERVO_RADAR
    );


    servoRadar.write(
        RADAR_CENTRO
    );


    Estado.radar.activo = true;


}




void Radar::actualizar()
{

    unsigned long ahora = millis();



    if(
        ahora - tiempoAnterior
        <
        TIEMPO_LECTURA_RADAR
    )
    {
        return;
    }



    tiempoAnterior = ahora;



    switch(estadoActual)
    {



        case EstadoRadarInterno::MOVER_IZQUIERDA:


            moverServo(
                RADAR_IZQUIERDA
            );


            Estado.radar.posicionActual =
                RADAR_IZQUIERDA;


            estadoActual =
                EstadoRadarInterno::LEER_IZQUIERDA;


        break;



        case EstadoRadarInterno::LEER_IZQUIERDA:


            Estado.sensores.distanciaIzquierda =
                Sensores::leerDistancia(
                    TRIG_RADAR,
                    ECHO_RADAR
                );


            estadoActual =
                EstadoRadarInterno::MOVER_CENTRO;


        break;



        case EstadoRadarInterno::MOVER_CENTRO:


            moverServo(
                RADAR_CENTRO
            );


            Estado.radar.posicionActual =
                RADAR_CENTRO;


            estadoActual =
                EstadoRadarInterno::LEER_CENTRO;


        break;



        case EstadoRadarInterno::LEER_CENTRO:


            Estado.sensores.distanciaCentro =
                Sensores::leerDistancia(
                    TRIG_RADAR,
                    ECHO_RADAR
                );


            estadoActual =
                EstadoRadarInterno::MOVER_DERECHA;


        break;



        case EstadoRadarInterno::MOVER_DERECHA:


            moverServo(
                RADAR_DERECHA
            );


            Estado.radar.posicionActual =
                RADAR_DERECHA;


            estadoActual =
                EstadoRadarInterno::LEER_DERECHA;


        break;



        case EstadoRadarInterno::LEER_DERECHA:


            Estado.sensores.distanciaDerecha =
                Sensores::leerDistancia(
                    TRIG_RADAR,
                    ECHO_RADAR
                );


            Estado.radar.lecturaValida = true;



            estadoActual =
                EstadoRadarInterno::MOVER_IZQUIERDA;


        break;


    }



}



void Radar::moverServo(
    uint8_t posicion
)
{

    servoRadar.write(
        posicion
    );


}



#endif