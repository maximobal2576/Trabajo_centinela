#ifndef ESTADO_H
#define ESTADO_H


#include <Arduino.h>

#include "Configuracion.h"



/*
======================================================

                CENTINELA V2

                   ESTADO

  Memoria central del sistema

======================================================
*/



//=====================================================
// ESTADO DEL USUARIO
//=====================================================

enum class EstadoUsuario
{

    DETENIDO,

    CAMINANDO,

    MOVIMIENTO_DESCONOCIDO

};




//=====================================================
// ESTADO DE ALIMENTACION
//=====================================================

enum class EstadoEnergia
{

    BATERIA,

    CARGANDO,

    CARGA_COMPLETA,

    BAJO_CONSUMO

};




//=====================================================
// ESTADO DE SENSORES
//=====================================================

struct EstadoSensores
{


    // Radar

    uint16_t radar30 = 0;

    uint16_t radar60 = 0;

    uint16_t radar90 = 0;

    uint16_t radar120 = 0;

    uint16_t radar150 = 0;

    bool radarOK = false;

    bool sensorRadarOK = false;



    // Sensor frontal

    uint16_t distanciaFrontal = 0;

    bool sensorFrontalOK = false;



    // Sensor suelo

    uint16_t distanciaSuelo = 0;

    bool sensorSueloOK = false;



    // MPU6050

    bool caidaDetectada = false;



};




//=====================================================
// ESTADO DEL RADAR
//=====================================================

struct EstadoRadar
{


    bool activo = false;


    uint8_t posicionActual = RADAR_CENTRO;


    bool lecturaValida = false;


};




//=====================================================
// ESTADO DE SALIDA
//=====================================================

struct EstadoSalida
{


    uint8_t motorIzquierdo = VIBRACION_APAGADA;


    uint8_t motorCentro = VIBRACION_APAGADA;


    uint8_t motorDerecho = VIBRACION_APAGADA;


    uint8_t motorFrontal = VIBRACION_APAGADA;


};




//=====================================================
// ESTADO DEL SISTEMA
//=====================================================

struct EstadoSistema
{


    ModoSistema modo =
        ModoSistema::NORMAL;



    EstadoUsuario usuario =
        EstadoUsuario::DETENIDO;



    EstadoEnergia energia =
        EstadoEnergia::BATERIA;



    bool sistemaSeguro = true;



    bool errorSensor = false;



    bool errorRadar = false;



};




//=====================================================
// ESTADO GENERAL CENTINELA
//=====================================================

struct EstadoCentinela
{


    EstadoSensores sensores;


    EstadoRadar radar;


    EstadoSalida salida;


    EstadoSistema sistema;


};




// Variable global

extern EstadoCentinela Estado;



#endif