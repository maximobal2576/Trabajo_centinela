#ifndef SENSORES_CPP
#define SENSORES_CPP


#include "Sensores.h"

#include "Configuracion.h"

#include "Estado.h"



/*
======================================================

                CENTINELA V2

                 SENSORES V2

======================================================
*/



void Sensores::iniciar()
{


    pinMode(
        TRIG_RADAR,
        OUTPUT
    );

    pinMode(
        ECHO_RADAR,
        INPUT
    );



    pinMode(
        TRIG_FRONTAL,
        OUTPUT
    );

    pinMode(
        ECHO_FRONTAL,
        INPUT
    );



    pinMode(
        TRIG_SUELO,
        OUTPUT
    );

    pinMode(
        ECHO_SUELO,
        INPUT
    );



    digitalWrite(
        TRIG_RADAR,
        LOW
    );


    digitalWrite(
        TRIG_FRONTAL,
        LOW
    );


    digitalWrite(
        TRIG_SUELO,
        LOW
    );


}





void Sensores::actualizar()
{


    //==============================
    // SENSOR FRONTAL
    //==============================


    uint16_t frontal =
        filtrarLectura(
            TRIG_FRONTAL,
            ECHO_FRONTAL
        );



    if(frontal < DISTANCIA_MAXIMA)
    {

        Estado.sensores.distanciaFrontal =
            frontal;


        Estado.sensores.sensorFrontalOK =
            true;

    }
    else
    {

        Estado.sensores.sensorFrontalOK =
            false;

    }





    //==============================
    // SENSOR SUELO
    //==============================


    uint16_t suelo =
        filtrarLectura(
            TRIG_SUELO,
            ECHO_SUELO
        );



    if(suelo < DISTANCIA_MAXIMA)
    {

        Estado.sensores.distanciaSuelo =
            suelo;


        Estado.sensores.sensorSueloOK =
            true;

    }
    else
    {

        Estado.sensores.sensorSueloOK =
            false;

    }







    if(DEBUG_SENSORES)
    {


        Serial.print(
            "Frontal: "
        );


        Serial.print(
            Estado.sensores.distanciaFrontal
        );


        Serial.print(
            " | Suelo: "
        );


        Serial.println(
            Estado.sensores.distanciaSuelo
        );


    }


}






uint16_t Sensores::leerDistancia(
    uint8_t trig,
    uint8_t echo
)
{


    digitalWrite(
        trig,
        LOW
    );


    delayMicroseconds(2);



    digitalWrite(
        trig,
        HIGH
    );


    delayMicroseconds(10);



    digitalWrite(
        trig,
        LOW
    );



    unsigned long tiempo =
        pulseIn(
            echo,
            HIGH,
            30000
        );



    // Sin respuesta

    if(tiempo == 0)
    {

        return DISTANCIA_MAXIMA;

    }



    uint16_t distancia =
        tiempo * 0.0343 / 2;



    if(
        distancia > DISTANCIA_MAXIMA
    )
    {

        distancia =
            DISTANCIA_MAXIMA;

    }



    return distancia;


}






uint16_t Sensores::filtrarLectura(
    uint8_t trig,
    uint8_t echo
)
{


    uint32_t suma = 0;



    constexpr uint8_t MUESTRAS = 1;



    for(
        uint8_t i = 0;
        i < MUESTRAS;
        i++
    )
    {


        suma += leerDistancia(
            trig,
            echo
        );


    }



    return suma / MUESTRAS;


}





#endif