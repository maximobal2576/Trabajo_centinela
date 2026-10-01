/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Movimiento.h
 * Descripcion: Lectura del MPU-6050 para deteccion de caida.
 *   v2: usa libreria MPU6050_light compatible con ESP32 via I2C nativo.
 **************************************************************************/
#ifndef MOVIMIENTO_H
#define MOVIMIENTO_H

class Movimiento {
public:
    static void iniciar();
    static void actualizar();

private:
    static bool _mpuOk;
    static bool _detectarCaida();
};

#endif // MOVIMIENTO_H
