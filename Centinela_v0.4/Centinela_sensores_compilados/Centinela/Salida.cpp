#ifndef SALIDA_CPP
#define SALIDA_CPP


#include "Salida.h"

#include "Configuracion.h"

#include "Estado.h"



/*
======================================================

                CENTINELA V2

                SALIDA V2

======================================================
*/





void Salida::iniciar()
{


    pinMode(
        MOTOR_IZQUIERDO,
        OUTPUT
    );


    pinMode(
        MOTOR_CENTRO,
        OUTPUT
    );


    pinMode(
        MOTOR_DERECHO,
        OUTPUT
    );


    pinMode(
        MOTOR_FRONTAL,
        OUTPUT
    );



    analogWrite(
        MOTOR_IZQUIERDO,
        0
    );


    analogWrite(
        MOTOR_CENTRO,
        0
    );


    analogWrite(
        MOTOR_DERECHO,
        0
    );


    analogWrite(
        MOTOR_FRONTAL,
        0
    );


}






void Salida::actualizar()
{


    // Ejecutar órdenes actuales


    controlarMotor(
        MOTOR_IZQUIERDO,
        Estado.salida.motorIzquierdo
    );



    controlarMotor(
        MOTOR_CENTRO,
        Estado.salida.motorCentro
    );



    controlarMotor(
        MOTOR_DERECHO,
        Estado.salida.motorDerecho
    );



    controlarMotor(
        MOTOR_FRONTAL,
        Estado.salida.motorFrontal
    );



    /*
    
    Después de ejecutar,
    limpiamos las órdenes.

    Si hay otro peligro,
    Decision volverá a escribir.

    */


    Estado.salida.motorIzquierdo =
        VIBRACION_APAGADA;



    Estado.salida.motorCentro =
        VIBRACION_APAGADA;



    Estado.salida.motorDerecho =
        VIBRACION_APAGADA;



    Estado.salida.motorFrontal =
        VIBRACION_APAGADA;



}






void Salida::controlarMotor(
    uint8_t pin,
    uint8_t intensidad
)
{


    // Seguridad

    if(
        intensidad >
        VIBRACION_MAXIMA
    )
    {

        intensidad =
            VIBRACION_MAXIMA;

    }



    analogWrite(
        pin,
        intensidad
    );


}





#endif