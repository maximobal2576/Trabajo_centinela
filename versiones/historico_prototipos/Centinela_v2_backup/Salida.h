/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Salida.h
 * Descripcion:
 *   Control PWM de 3 motores vibradores via LEDC del ESP32.
 **************************************************************************/
#ifndef SALIDA_H
#define SALIDA_H

class Salida {
public:
    static void iniciar();
    static void actualizar();

private:
    static void escribirMotor(int canal, int valor);
};

#endif // SALIDA_H
