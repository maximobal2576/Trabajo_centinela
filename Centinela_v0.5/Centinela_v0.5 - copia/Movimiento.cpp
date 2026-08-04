/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Movimiento.cpp
 * Descripción:
 * Lectura MPU6050 y detección de caída.
 **************************************************************************/


#include "Movimiento.h"

#include "Estado.h"

#include <Wire.h>
#include <math.h>



float Movimiento::aceleracionAnterior = 0;



void Movimiento::iniciar()
{

    Wire.begin();


    // Activar MPU6050

    Wire.beginTransmission(0x68);

    Wire.write(0x6B);

    Wire.write(0);

    Wire.endTransmission(true);


    Serial.println("MPU6050 iniciado");

}



void Movimiento::actualizar()
{


    // Pedimos acelerómetro

    Wire.beginTransmission(0x68);

    Wire.write(0x3B);

    Wire.endTransmission(false);


    Wire.requestFrom(0x68, 6, true);



    if(Wire.available() >= 6)
    {


        int16_t ax =
        Wire.read() << 8 | Wire.read();


        int16_t ay =
        Wire.read() << 8 | Wire.read();


        int16_t az =
        Wire.read() << 8 | Wire.read();



        float x = ax / 16384.0;

        float y = ay / 16384.0;

        float z = az / 16384.0;



        Estado::sensores.inclinacionX = x;

        Estado::sensores.inclinacionY = y;

        Estado::sensores.inclinacionZ = z;



        float modulo =
        sqrt(
            x*x +
            y*y +
            z*z
        );



        Estado::sensores.movimientoDetectado =
        abs(modulo - 1.0) > 0.3;



        Estado::sensores.caidaDetectada =
        detectarCaida();



        // Para pruebas:
        /*
        Serial.print("Aceleracion: ");
        Serial.println(modulo);
        */

    }



}



// =====================================================
// DETECCION DE CAIDA
// =====================================================

bool Movimiento::detectarCaida()
{


    float x =
    Estado::sensores.inclinacionX;


    float y =
    Estado::sensores.inclinacionY;


    float z =
    Estado::sensores.inclinacionZ;



    float modulo =
    sqrt(
        x*x +
        y*y +
        z*z
    );



    /*
       Quieto:
       aproximadamente 1G

       Golpe fuerte:
       >1.6G

       Caída libre:
       <0.5G
    */


    if(modulo < 0.5 || modulo > 1.6)
    {

        return true;

    }



    return false;

}