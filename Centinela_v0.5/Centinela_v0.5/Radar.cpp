#include "Radar.h"
#include "Configuracion.h"
#include <Servo.h>


namespace Radar
{


//======================================================
// Prototipos internos
//======================================================

int medirDistancia();

int filtrarZona(int zona, int nueva);



//======================================================
// Servo
//======================================================

Servo servo;



//======================================================
// Variables internas
//======================================================

int anguloActual = 20;

int direccion = 1;


int distanciaActual = 250;


unsigned long ultimoPaso = 0;



//======================================================
// Configuración
//======================================================

const int ANGULO_MIN = 20;

const int ANGULO_MAX = 160;


const int PASO = 5;


const unsigned long TIEMPO_PASO = 150;


const int TIEMPO_ESTABILIZACION = 100;



//======================================================
// Memoria de zonas
// 0 izquierda
// 1 diagonal izquierda
// 2 centro
// 3 diagonal derecha
// 4 derecha
//======================================================

int distanciaZona[5] =
{
    250,
    250,
    250,
    250,
    250
};


int filtroZona[5] =
{
    -1,
    -1,
    -1,
    -1,
    -1
};

unsigned long tiempoZona[5] =
{
    0,
    0,
    0,
    0,
    0
};


const unsigned long TIEMPO_MEMORIA = 1000;

//======================================================
// Inicialización
//======================================================

void iniciar()
{

    pinMode(TRIG_RADAR, OUTPUT);

    pinMode(ECHO_RADAR, INPUT);



    servo.attach(PIN_SERVO);


    servo.write(anguloActual);



    delay(300);

}



//======================================================
// Actualización radar
//======================================================

void actualizar()
{

    if(millis() - ultimoPaso < TIEMPO_PASO)
        return;



    ultimoPaso = millis();



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



    delay(TIEMPO_ESTABILIZACION);



    int lectura = medirDistancia();



    int zona = obtenerZona();



    distanciaZona[zona] =
    filtrarZona(zona, lectura);

    tiempoZona[zona] = millis();


    distanciaActual =
    distanciaZona[zona];



    Serial.print("Angulo: ");
    Serial.print(anguloActual);


    Serial.print(" Zona: ");
    Serial.print(zona);


    Serial.print(" Distancia: ");
    Serial.println(distanciaActual);

}



//======================================================
// Lectura HC-SR04
//======================================================

int medirDistancia()
{

    digitalWrite(TRIG_RADAR, LOW);

    delayMicroseconds(2);



    digitalWrite(TRIG_RADAR, HIGH);

    delayMicroseconds(10);



    digitalWrite(TRIG_RADAR, LOW);



    unsigned long tiempo =
    pulseIn(
        ECHO_RADAR,
        HIGH,
        50000
    );



    if(tiempo == 0)
        return 250;



    int distancia =
    tiempo * 0.034 / 2;



    if(distancia < 3 || distancia > 250)
        return 250;



    return distancia;

}



//======================================================
// Filtro individual por zona
//======================================================

int filtrarZona(int zona, int nueva)
{

    if(filtroZona[zona] == -1)
    {
        filtroZona[zona] = nueva;
        return nueva;
    }


    int diferencia = abs(nueva - filtroZona[zona]);


    // Si cambia mucho aceptamos nueva lectura
    if(diferencia > 25)
    {
        filtroZona[zona] = nueva;
        return nueva;
    }


    // Promedio más suave
    filtroZona[zona] =
    (filtroZona[zona] * 0.3) +
    (nueva * 0.7);


    return filtroZona[zona];

}



//======================================================
// Funciones públicas
//======================================================

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

    if(anguloActual < 48)
        return 0;



    if(anguloActual < 76)
        return 1;



    if(anguloActual < 104)
        return 2;



    if(anguloActual < 132)
        return 3;



    return 4;

}



int obtenerDistanciaZona(int zona)
{

    if(zona < 0 || zona > 4)
        return 250;


    if(millis() - tiempoZona[zona] > TIEMPO_MEMORIA)
    {
        return 250;
    }


    return distanciaZona[zona];

}



}