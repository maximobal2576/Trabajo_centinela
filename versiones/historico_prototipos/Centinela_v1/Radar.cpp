#include "Radar.h"
#include "Estado.h"
#include "Configuracion.h"

#include <Servo.h>


Servo servoRadar;


int anguloRadar = RADAR_IZQUIERDA;
int direccionBarrido = RADAR_PASO;



// ------------------------------
// INICIAR RADAR
// ------------------------------

void Radar::iniciar()
{

    servoRadar.attach(PIN_SERVO);


    pinMode(TRIG_RADAR, OUTPUT);
    pinMode(ECHO_RADAR, INPUT);



    servoRadar.write(anguloRadar);



    Estado::sensores.radarIzquierda = 400;
    Estado::sensores.radarCentro = 400;
    Estado::sensores.radarDerecha = 400;
    Estado::sensores.anguloRadar = anguloRadar;



    Serial.println("Radar iniciado");

}



// ------------------------------
// MEDIR DISTANCIA HC-SR04 (RÁPIDO)
// ------------------------------

int medirRadar()
{

    digitalWrite(TRIG_RADAR, LOW);
    delayMicroseconds(2);



    digitalWrite(TRIG_RADAR, HIGH);
    delayMicroseconds(10);



    digitalWrite(TRIG_RADAR, LOW);



    // Timeout de 12ms (~2 metros max) para no congelar el microcontrolador
    long tiempo = pulseIn(ECHO_RADAR, HIGH, 12000);
    if(tiempo <= 0)
    {

        return 400;

    }



    int distancia = (int)(tiempo * 0.034 / 2);
    if(distancia < 2 || distancia > 400)
    {

        return 400;

    }



    return distancia;

}



// ------------------------------
// ACTUALIZAR RADAR (BARRIDO FLUIDO)
// ------------------------------

void Radar::actualizar()
{

    servoRadar.write(anguloRadar);



    int distancia = medirRadar();



    // Guardar según dirección del ángulo actual
    if(anguloRadar <= 65)
    {

        Estado::sensores.radarIzquierda = distancia;

    }


    else if(anguloRadar <= 115)
    {

        Estado::sensores.radarCentro = distancia;

    }


    else
    {

        Estado::sensores.radarDerecha = distancia;

    }

    Estado::sensores.anguloRadar = anguloRadar;




    // Barrido ida y vuelta (ping-pong suave)
    anguloRadar += direccionBarrido;
    if(anguloRadar >= RADAR_DERECHA)
    {

        anguloRadar = RADAR_DERECHA;

        direccionBarrido = -RADAR_PASO;

    }


    else if(anguloRadar <= RADAR_IZQUIERDA)
    {

        anguloRadar = RADAR_IZQUIERDA;

        direccionBarrido = RADAR_PASO;

    }


}
