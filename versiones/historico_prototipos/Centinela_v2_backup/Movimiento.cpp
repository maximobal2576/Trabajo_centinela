/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Movimiento.cpp
 * Descripcion:
 *   Lectura MPU-6050 para deteccion de caida libre / impacto.
 *   Usa MPU6050_light que funciona con Wire (I2C nativo ESP32).
 *   Si el sensor no esta conectado, el modulo se deshabilita sin crash.
 **************************************************************************/
#include "Movimiento.h"
#include "Estado.h"
#include "Configuracion.h"

#include <Wire.h>
#include <MPU6050_light.h>

static MPU6050 mpu(Wire);
bool Movimiento::_mpuOk = false;

void Movimiento::iniciar()
{
    Wire.begin(MPU_SDA, MPU_SCL);

    byte status = mpu.begin();
    if (status == 0)
    {
        _mpuOk = true;
        mpu.calcOffsets(true, true);   // Calibracion automatica (baston quieto al encender)
        Serial.println("MPU-6050 OK");
    }
    else
    {
        _mpuOk = false;
        Serial.println("MPU-6050 NO ENCONTRADO (revisar conexion)");
    }

    Estado::sensores.inclinacionX      = 0;
    Estado::sensores.inclinacionY      = 0;
    Estado::sensores.inclinacionZ      = 0;
    Estado::sensores.movimientoDetectado = false;
    Estado::sensores.caidaDetectada    = false;
}

void Movimiento::actualizar()
{
    if (!_mpuOk) return;

    mpu.update();

    Estado::sensores.inclinacionX = mpu.getAccX();
    Estado::sensores.inclinacionY = mpu.getAccY();
    Estado::sensores.inclinacionZ = mpu.getAccZ();

    Estado::sensores.caidaDetectada = _detectarCaida();
}

bool Movimiento::_detectarCaida()
{
    float x = Estado::sensores.inclinacionX;
    float y = Estado::sensores.inclinacionY;
    float z = Estado::sensores.inclinacionZ;

    float modulo = sqrt(x*x + y*y + z*z);

    // Lectura invalida si sensor desconectado
    if (modulo < 0.15f) return false;

    // Caida libre (< 0.45G) o golpe fuerte (> 1.75G)
    return (modulo < MPU_UMBRAL_CAIDA_LIBRE || modulo > MPU_UMBRAL_GOLPE);
}
