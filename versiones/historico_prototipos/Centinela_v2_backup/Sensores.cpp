/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Sensores.cpp
 * Descripcion:
 *   Lectura de sensores HC-SR04.
 *   pulseIn() funciona correctamente en ESP32.
 *   Los pines han sido actualizados para el ESP32 (ver Configuracion.h).
 **************************************************************************/
#include "Sensores.h"
#include "Estado.h"
#include "Configuracion.h"

void Sensores::iniciar()
{
    pinMode(TRIG_FRONTAL, OUTPUT);
    pinMode(ECHO_FRONTAL, INPUT);

    pinMode(TRIG_SUELO, OUTPUT);
    pinMode(ECHO_SUELO, INPUT);

    pinMode(TRIG_RADAR, OUTPUT);
    pinMode(ECHO_RADAR, INPUT);

    digitalWrite(TRIG_FRONTAL, LOW);
    digitalWrite(TRIG_SUELO,   LOW);
    digitalWrite(TRIG_RADAR,   LOW);
}

void Sensores::actualizar()
{
    Estado::sensores.frontal = leerDistancia(TRIG_FRONTAL, ECHO_FRONTAL);
    Estado::sensores.suelo   = leerDistancia(TRIG_SUELO,   ECHO_SUELO);
    // El radar se gestiona desde Radar.cpp para mantener una sola
    // responsabilidad y evitar mediciones cruzadas con el servo.
}

int Sensores::leerDistancia(int trig, int echo)
{
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    // Timeout 15 ms (~2.5 m maximos)
    unsigned long duracion = pulseIn(echo, HIGH, 15000UL);
    if (duracion == 0) return 400;

    int distancia = (int)(duracion * 0.034f / 2.0f);
    if (distancia < 2 || distancia > 400) distancia = 400;

    return distancia;
}
