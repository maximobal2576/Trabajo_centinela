/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Salida.cpp
 * Descripción:
 * Control PWM de motores vibradores.
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



    analogWrite(MOTOR_IZQUIERDO, 0);

    analogWrite(MOTOR_CENTRO, 0);

    analogWrite(MOTOR_DERECHO, 0);

    analogWrite(MOTOR_FRONTAL, 0);

}



void Salida::actualizar()
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


    actualizarBuzzer();
}



void Salida::escribirMotor(int pin, int valor)
{


    valor = constrain(valor, 0, 255);


    analogWrite(pin, valor);


}
void Salida::actualizarBuzzer()
{

    static unsigned long tiempoBuzzer = 0;

    static bool buzzerActivo = false;

    unsigned long periodo = 0;


    if(Estado::sistema.eventoActual == EVENTO_CAIDA)
    {
        periodo = 250; // Beep rápido de emergencia
    }
    else if(Estado::salida.buzzerBalizaGPS)
    {
        periodo = 180; // Baliza acústica GPS de esquina (a 3 metros)
    }
    else if(Estado::sistema.eventoActual == EVENTO_BATERIA_BAJA)
    {
        periodo = 700;
    }
    else if(Estado::sistema.eventoActual == EVENTO_ERROR)
    {
        periodo = 500;
    }

    if(periodo > 0)
    {
        if(millis() - tiempoBuzzer >= periodo)
        {
            tiempoBuzzer = millis();

            buzzerActivo = !buzzerActivo;

            digitalWrite(
                BUZZER_PIN,
                buzzerActivo
            );
        }
    }
    else
    {
        buzzerActivo = false;

        digitalWrite(
            BUZZER_PIN,
            LOW
        );
    }

}