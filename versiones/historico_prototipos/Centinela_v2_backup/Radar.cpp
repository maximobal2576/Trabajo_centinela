/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Radar.cpp
 * Descripcion:
 *   Barrido con servo SG90 y sensor HC-SR04 inferior.
 *   ESP32Servo usa el timer LEDC interno del ESP32.
 *   Se usan los mismos canales LEDC que los motores pero en frecuencia
 *   diferente; ESP32Servo asigna su propio canal automaticamente.
 **************************************************************************/
#include "Radar.h"
#include "Estado.h"
#include "Configuracion.h"
#include "Sensores.h"

Servo Radar::servo;
int   Radar::_angulo           = RADAR_IZQUIERDA;
int   Radar::_direccionBarrido = 1;

void Radar::iniciar()
{
    // ESP32Servo libera un canal LEDC automaticamente
    servo.attach(PIN_SERVO);
    servo.write(RADAR_CENTRO);
    delay(300);

    Estado::sensores.anguloRadar     = RADAR_CENTRO;
    Estado::sensores.radarIzquierda  = 400;
    Estado::sensores.radarCentro     = 400;
    Estado::sensores.radarDerecha    = 400;

    _angulo = RADAR_IZQUIERDA;
}

void Radar::actualizar()
{
    _moverSiguiente();
}

void Radar::_moverSiguiente()
{
    _angulo += RADAR_PASO * _direccionBarrido;

    if (_angulo >= RADAR_DERECHA)
    {
        _angulo = RADAR_DERECHA;
        _direccionBarrido = -1;
    }
    else if (_angulo <= RADAR_IZQUIERDA)
    {
        _angulo = RADAR_IZQUIERDA;
        _direccionBarrido = 1;
    }

    servo.write(_angulo);
    Estado::sensores.anguloRadar = _angulo;

    int dist = _medirEnAngulo(_angulo);

    // Asignar la medicion a la zona correspondiente
    if (_angulo <= (RADAR_IZQUIERDA + (RADAR_CENTRO - RADAR_IZQUIERDA) / 2))
    {
        Estado::sensores.radarIzquierda = dist;
    }
    else if (_angulo <= (RADAR_CENTRO + (RADAR_DERECHA - RADAR_CENTRO) / 2))
    {
        Estado::sensores.radarCentro = dist;
    }
    else
    {
        Estado::sensores.radarDerecha = dist;
    }
}

int Radar::_medirEnAngulo(int angulo)
{
    return Sensores::leerDistancia(TRIG_RADAR, ECHO_RADAR);
}
