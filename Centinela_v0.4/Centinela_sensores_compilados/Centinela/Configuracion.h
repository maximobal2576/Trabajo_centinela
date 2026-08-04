#ifndef CONFIGURACION_H
#define CONFIGURACION_H


#include <Arduino.h>


/*
======================================================
                CENTINELA V2
            CONFIGURACION PRINCIPAL

  Archivo:
  Configuracion.h

  Funcion:
  Contiene todos los parametros configurables
  del sistema.

======================================================
*/


//=====================================================
// VERSION DEL FIRMWARE
//=====================================================

constexpr uint8_t VERSION_MAYOR = 2;
constexpr uint8_t VERSION_MENOR = 0;
constexpr uint8_t VERSION_REVISION = 0;


//=====================================================
// MODOS DEL SISTEMA
//=====================================================

enum class ModoSistema
{

    NORMAL,

    CARGANDO,

    ESPERA,

    PRUEBA,

    DIAGNOSTICO,

    CALIBRACION,

    BAJO_CONSUMO,

    SEGURO

};


//=====================================================
// PINES RADAR
//=====================================================

constexpr uint8_t PIN_SERVO_RADAR = 10;


//=====================================================
// PINES SENSORES ULTRASONICOS
//=====================================================

// Radar
constexpr uint8_t TRIG_RADAR = 22;
constexpr uint8_t ECHO_RADAR = 23;


// Frontal
constexpr uint8_t TRIG_FRONTAL = 24;
constexpr uint8_t ECHO_FRONTAL = 25;


// Suelo
constexpr uint8_t TRIG_SUELO = 26;
constexpr uint8_t ECHO_SUELO = 27;



//=====================================================
// PINES MOTORES VIBRADORES
//=====================================================

constexpr uint8_t MOTOR_IZQUIERDO = 5;

constexpr uint8_t MOTOR_CENTRO = 6;

constexpr uint8_t MOTOR_DERECHO = 9;

constexpr uint8_t MOTOR_FRONTAL = 11;



//=====================================================
// CONFIGURACION RADAR
//=====================================================


constexpr uint8_t RADAR_IZQUIERDA_EXTREMA = 30;

constexpr uint8_t RADAR_IZQUIERDA = 60;

constexpr uint8_t RADAR_CENTRO = 90;

constexpr uint8_t RADAR_DERECHA = 120;

constexpr uint8_t RADAR_DERECHA_EXTREMA = 150;



// Tiempo entre movimientos del radar

constexpr uint16_t TIEMPO_LECTURA_RADAR = 120;



//=====================================================
// DISTANCIAS DE SEGURIDAD (cm)
//=====================================================

constexpr uint16_t DISTANCIA_CRITICA = 40;

constexpr uint16_t DISTANCIA_PRECAUCION = 90;

constexpr uint16_t DISTANCIA_MAXIMA = 250;



//=====================================================
// VIBRACIONES
//=====================================================

constexpr uint8_t VIBRACION_APAGADA = 0;

constexpr uint8_t VIBRACION_SUAVE = 80;

constexpr uint8_t VIBRACION_MEDIA = 140;

constexpr uint8_t VIBRACION_FUERTE = 220;

constexpr uint8_t VIBRACION_MAXIMA = 255;



//=====================================================
// MPU6050
//=====================================================

constexpr float ACELERACION_CAIDA = 2.5;


//=====================================================
// TEMPORIZADORES GENERALES
//=====================================================

constexpr uint16_t TIEMPO_LECTURA_SENSORES = 60;

constexpr uint16_t TIEMPO_DIAGNOSTICO = 1000;



//=====================================================
// DEBUG
//=====================================================

constexpr bool DEBUG_GENERAL = true;

constexpr bool DEBUG_RADAR = true;

constexpr bool DEBUG_SENSORES = true;

constexpr bool DEBUG_MPU = false;



#endif