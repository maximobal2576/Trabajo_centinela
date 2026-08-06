#include "Radar.h"
#include "Configuracion.h"



namespace Radar
{


Servo servo;



// Estado interno

int anguloActual = 20;

int direccion = 1;


int distanciaActual = 250;


unsigned long ultimoPaso = 0;



// Configuración radar

const int ANGULO_MIN = 20;
const int ANGULO_MAX = 160;


const int PASO = 5;


const unsigned long TIEMPO_PASO = 150;


const int TIEMPO_ESTABILIZACION = 100;



int distanciaAnterior = -1;



void iniciar()
{


    pinMode(TRIG_RADAR, OUTPUT);

    pinMode(ECHO_RADAR, INPUT);



    servo.attach(PIN_SERVO);


    servo.write(anguloActual);
    


    delay(300);



}



void actualizar()
{
    Serial.print("Radar angulo: ");
    Serial.print(anguloActual);

    Serial.print(" distancia: ");
    Serial.println(distanciaActual);

    if(millis() - ultimoPaso < TIEMPO_PASO)
        return;



    ultimoPaso = millis();



    // Movimiento del radar

    anguloActual += PASO * direccion;



    if(anguloActual >= ANGULO_MAX)
    {
        anguloActual = ANGULO_MAX;
        direccion = -1;
    }



    if(anguloActual <= ANGULO_MIN)
    {
        anguloActual = ANGULO_MIN;
        direccion = 1;
    }

    servo.write(anguloActual);


    Serial.print("Moviendo a: ");
    Serial.println(anguloActual);


    delay(TIEMPO_ESTABILIZACION);


    int lectura = medirDistancia();


    distanciaActual = filtrar(lectura);



    Serial.print("Lectura cruda: ");
    Serial.print(lectura);

    Serial.print("  Filtrada: ");
    Serial.println(distanciaActual);
    }





int medirDistancia()
{


    digitalWrite(TRIG_RADAR, LOW);

    delayMicroseconds(2);



    digitalWrite(TRIG_RADAR, HIGH);

    delayMicroseconds(10);



    digitalWrite(TRIG_RADAR, LOW);



    unsigned long tiempo = pulseIn(
        ECHO_RADAR,
        HIGH,
        50000
    );


    if(tiempo == 0)
        return 250;



    int distancia = tiempo * 0.034 / 2;



    if(distancia < 3 || distancia > 250)
        return 250;



    return distancia;


}





int filtrar(int nueva)
{

    if(distanciaAnterior == -1)
    {
        distanciaAnterior = nueva;
        return nueva;
    }


    int diferencia = abs(nueva - distanciaAnterior);


    // Cambio grande: probablemente es un obstáculo real
    if(diferencia > 60)
    {
        distanciaAnterior = nueva;
        return nueva;
    }


    // Cambio pequeño: suavizar
    distanciaAnterior =
    (distanciaAnterior * 0.6) +
    (nueva * 0.4);


    return distanciaAnterior;

}




int obtenerDistancia()
{
    return distanciaActual;
}





int obtenerAngulo()
{
    return anguloActual;
}





int obtenerZona()
{


    if(anguloActual < 66)
        return 0;   // izquierda



    if(anguloActual < 116)
        return 1;   // centro



    return 2;       // derecha


}



}