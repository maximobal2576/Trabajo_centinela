/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Sensores.cpp
 * Descripción:
 * Lectura de sensores HC-SR04.
 **************************************************************************/


#include "Sensores.h"

#include "Estado.h"



void Sensores::iniciar()
{

    pinMode(TRIG_FRONTAL, OUTPUT);
    pinMode(ECHO_FRONTAL, INPUT);


    pinMode(TRIG_SUELO, OUTPUT);
    pinMode(ECHO_SUELO, INPUT);


    pinMode(TRIG_RADAR, OUTPUT);
    pinMode(ECHO_RADAR, INPUT);



    digitalWrite(TRIG_FRONTAL, LOW);

    digitalWrite(TRIG_SUELO, LOW);

    digitalWrite(TRIG_RADAR, LOW);

}



void Sensores::actualizar()
{


    Estado::sensores.frontal =
        leerDistancia(TRIG_FRONTAL, ECHO_FRONTAL);



    Estado::sensores.suelo =
        leerDistancia(TRIG_SUELO, ECHO_SUELO);



    // Radar temporalmente en centro
    // Luego será controlado por Radar.cpp

    Estado::sensores.radarCentro =
        leerDistancia(TRIG_RADAR, ECHO_RADAR);



}



int Sensores::leerDistancia(int trig, int echo)
{


    digitalWrite(trig, LOW);

    delayMicroseconds(2);


    digitalWrite(trig, HIGH);

    delayMicroseconds(10);


    digitalWrite(trig, LOW);



    unsigned long duracion =
        pulseIn(echo, HIGH, 30000);



    if (duracion == 0)
    {

        return 400;

    }



    int distancia =
        duracion * 0.034 / 2;



    if (distancia > 400)
    {

        distancia = 400;

    }



    return distancia;

}