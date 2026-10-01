/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Salida.cpp
 * Descripción:
 * Control de salidas físicas del sistema.
 **************************************************************************/

#include "Salida.h"

#include "Estado.h"



void Salida::iniciar()
{

    pinMode(MOTOR_IZQUIERDO, OUTPUT);

    pinMode(MOTOR_CENTRO, OUTPUT);

    pinMode(MOTOR_DERECHO, OUTPUT);

    pinMode(MOTOR_FRONTAL, OUTPUT);


    pinMode(BUZZER_PIN, OUTPUT);


    digitalWrite(BUZZER_PIN, LOW);



    analogWrite(MOTOR_IZQUIERDO,0);

    analogWrite(MOTOR_CENTRO,0);

    analogWrite(MOTOR_DERECHO,0);

    analogWrite(MOTOR_FRONTAL,0);

}





void Salida::actualizar()
{


    // Seguridad: caída tiene prioridad absoluta

    if(Estado::sensores.caidaDetectada)
    {

        escribirMotor(MOTOR_IZQUIERDO,0);

        escribirMotor(MOTOR_CENTRO,0);

        escribirMotor(MOTOR_DERECHO,0);

        escribirMotor(MOTOR_FRONTAL,0);


    }
    else
    {


        escribirMotor(
            MOTOR_IZQUIERDO,
            Estado::salida.motorIzquierdo
        );


        escribirMotor(
            MOTOR_CENTRO,
            Estado::salida.motorCentro
        );


        escribirMotor(
            MOTOR_DERECHO,
            Estado::salida.motorDerecho
        );


        escribirMotor(
            MOTOR_FRONTAL,
            Estado::salida.motorFrontal
        );


    }



    actualizarBuzzer();

}





void Salida::escribirMotor(int pin, int valor)
{

    valor = constrain(valor,0,255);


    analogWrite(pin,valor);

}






void Salida::actualizarBuzzer()
{


    static unsigned long tiempoBuzzer = 0;

    static bool estadoBuzzer = false;




    if(Estado::sensores.caidaDetectada)
    {


        if(millis() - tiempoBuzzer >= 300)
        {

            tiempoBuzzer = millis();


            estadoBuzzer = !estadoBuzzer;


            digitalWrite(
                BUZZER_PIN,
                estadoBuzzer
            );

        }

    }
    else
    {


        estadoBuzzer = false;


        digitalWrite(
            BUZZER_PIN,
            LOW
        );


    }


}