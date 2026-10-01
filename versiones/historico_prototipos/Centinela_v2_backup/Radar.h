/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Radar.h
 * Descripcion: Servo + HC-SR04 inferior (barrido iz-centro-der).
 *   v2: usa ESP32Servo en lugar de Servo.h estandar.
 **************************************************************************/
#ifndef RADAR_H
#define RADAR_H

#include <ESP32Servo.h>

class Radar {
public:
    static void iniciar();
    static void actualizar();

private:
    static Servo   servo;
    static int     _angulo;
    static int     _direccionBarrido;   // +1 derecha, -1 izquierda

    static void _moverSiguiente();
    static int  _medirEnAngulo(int angulo);
};

#endif // RADAR_H
