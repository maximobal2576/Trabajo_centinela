#include "Radar.h"
#include "Estado.h"
#include "Configuracion.h"

#include <Servo.h>


Servo servoRadar;


int anguloRadar = 45;



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



    Serial.println("Radar iniciado");

}



// ------------------------------
// MEDIR DISTANCIA HC-SR04
// ------------------------------

int medirRadar()
{

    long suma = 0;

    int validas = 0;



    for(int i = 0; i < 5; i++)
    {


        digitalWrite(TRIG_RADAR, LOW);
        delayMicroseconds(2);



        digitalWrite(TRIG_RADAR, HIGH);
        delayMicroseconds(10);



        digitalWrite(TRIG_RADAR, LOW);



        long tiempo =
        pulseIn(ECHO_RADAR, HIGH, 30000);



        if(tiempo > 0)
        {


            int distancia =
            tiempo * 0.034 / 2;



            if(distancia >= 2 && distancia <=400)
            {

                suma += distancia;

                validas++;

            }

        }



        delay(10);

    }



    if(validas == 0)
    {

        return 400;

    }



    return suma / validas;

}



// ------------------------------
// ACTUALIZAR RADAR
// ------------------------------

void Radar::actualizar()
{


    static unsigned long tiempo = 0;



    // velocidad del barrido

    if(millis() - tiempo < 200)
    {
        return;
    }



    tiempo = millis();



    // mover servo

    servoRadar.write(anguloRadar);



    // esperar estabilización

    delay(100);



    int distancia = medirRadar();




    Serial.print("Angulo: ");

    Serial.print(anguloRadar);


    Serial.print("  Distancia: ");

    Serial.print(distancia);


    Serial.println(" cm");




    // -------------------------
    // GUARDAR SEGÚN DIRECCIÓN
    // -------------------------


    if(anguloRadar <= 75)
    {

        Estado::sensores.radarIzquierda = distancia;

    }


    else if(anguloRadar <=105)
    {

        Estado::sensores.radarCentro = distancia;

    }


    else
    {

        Estado::sensores.radarDerecha = distancia;

    }




    Serial.print("IZQ: ");

    Serial.print(Estado::sensores.radarIzquierda);



    Serial.print("  CENTRO: ");

    Serial.print(Estado::sensores.radarCentro);



    Serial.print("  DER: ");

    Serial.println(Estado::sensores.radarDerecha);




    // avanzar servo


    anguloRadar += 10;



    if(anguloRadar > 135)
    {

        anguloRadar = 45;

    }


}