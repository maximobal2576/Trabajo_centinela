/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Movimiento.cpp
 * Descripción:
 * Lectura MPU6050 y detección de caída y movimiento.
 **************************************************************************/

#include "Movimiento.h"
#include "Estado.h"
#include <Wire.h>
#include <math.h>

#define MPU_ADDR 0x68

float Movimiento::aceleracionAnterior = 1.0;
unsigned long tiempoCaidaInicio_v1 = 0;
const unsigned long TIEMPO_RECUPERACION_CAIDA_V1 = 5000;
static uint8_t direccionMPU = MPU_ADDR;
static bool mpuDisponible = false;

void Movimiento::iniciar()
{
    Wire.begin();
    Wire.setWireTimeout(3000, true); // Timeout de 3ms para no colgar el I2C

    // Probar dirección 0x68 (por defecto) o 0x69
    direccionMPU = 0x68;
    Wire.beginTransmission(direccionMPU);
    if(Wire.endTransmission() != 0)
    {
        direccionMPU = 0x69;
    }

    // Despertar MPU6050 y configurar reloj
    Wire.beginTransmission(direccionMPU);
    Wire.write(0x6B); // PWR_MGMT_1
    Wire.write(0x00); // Salir de sleep
    if(Wire.endTransmission() == 0)
    {
        mpuDisponible = true;
        Serial.print("MPU6050 detectado en 0x");
        Serial.println(direccionMPU, HEX);

        // Configurar rango acelerómetro: +-4g (8192 LSB/g)
        Wire.beginTransmission(direccionMPU);
        Wire.write(0x1C); // ACCEL_CONFIG
        Wire.write(0x08); // +-4g
        Wire.endTransmission();

        // Configurar filtro DLPF 44Hz
        Wire.beginTransmission(direccionMPU);
        Wire.write(0x1A); // CONFIG
        Wire.write(0x03);
        Wire.endTransmission();
    }
    else
    {
        mpuDisponible = false;
        Serial.println("MPU6050 no responde en I2C");
    }
}

void Movimiento::actualizar()
{
    if(!mpuDisponible)
    {
        // Reintentar conexión si no estaba disponible
        iniciar();
        if(!mpuDisponible) return;
    }

    // Leer 6 bytes del acelerómetro (0x3B a 0x40)
    Wire.beginTransmission(direccionMPU);
    Wire.write(0x3B);
    Wire.endTransmission(true);

    Wire.requestFrom((uint8_t)direccionMPU, (uint8_t)6, (uint8_t)true);

    if(Wire.available() >= 6)
    {
        int16_t ax = (Wire.read() << 8) | Wire.read();
        int16_t ay = (Wire.read() << 8) | Wire.read();
        int16_t az = (Wire.read() << 8) | Wire.read();

        // Con rango +-4g, el factor es 8192.0 LSB/g
        float x = ax / 8192.0;
        float y = ay / 8192.0;
        float z = az / 8192.0;

        Estado::sensores.inclinacionX = x;
        Estado::sensores.inclinacionY = y;
        Estado::sensores.inclinacionZ = z;

        float modulo = sqrt(x*x + y*y + z*z);

        // Movimiento: si hay variación respecto a 1G o cambio de aceleración
        float delta = abs(modulo - Movimiento::aceleracionAnterior);
        Movimiento::aceleracionAnterior = modulo;

        Estado::sensores.movimientoDetectado = (delta > 0.12) || (abs(modulo - 1.0) > 0.20);

        // Detección de caída / impacto fuerte
        bool detectada = detectarCaida();

        if(detectada && !Estado::sensores.caidaDetectada)
        {
            Estado::sensores.caidaDetectada = true;
            tiempoCaidaInicio_v1 = millis();
            Serial.println("¡ALERTA CAIDA DETECTADA!");
        }
        else if(Estado::sensores.caidaDetectada)
        {
            // Recuperación si el bastón vuelve a posición normal (~1G) y se mueve,
            // o tras pasar el tiempo de alerta de 5 segundos
            bool inclinacionNormal = (modulo > 0.75 && modulo < 1.25);
            if((inclinacionNormal && Estado::sensores.movimientoDetectado) || (millis() - tiempoCaidaInicio_v1 > TIEMPO_RECUPERACION_CAIDA_V1))
            {
                Estado::sensores.caidaDetectada = false;
                tiempoCaidaInicio_v1 = 0;
            }
        }
    }
}

// =====================================================
// DETECCION DE CAIDA
// =====================================================

bool Movimiento::detectarCaida()
{
    float x = Estado::sensores.inclinacionX;
    float y = Estado::sensores.inclinacionY;
    float z = Estado::sensores.inclinacionZ;

    float modulo = sqrt(x*x + y*y + z*z);

    // Lectura inválida si sensor desconectado
    if(modulo < 0.15)
    {
        return false;
    }

    // 1. Golpe fuerte / sacudida (> 1.7G)
    // 2. Caída libre momentánea (< 0.45G)
    if(modulo < 0.45 || modulo > 1.75)
    {
        return true;
    }

    return false;
}