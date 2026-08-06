#ifndef ESTADO_H
#define ESTADO_H


#include <Arduino.h>

#include "Configuracion.h"



/*
======================================================

                CENTINELA V2

                  ESTADO

  Archivo:
  Estado.h


  Funcion:

  Guarda el estado actual del sistema.

  Todos los módulos se comunican
  mediante esta estructura.

======================================================
*/



//=====================================================
// ESTADO DE SENSORES
//=====================================================

struct EstadoSensores
{

    // Radar
    uint16_t distanciaIzquierda = 0;

    uint16_t distanciaCentro = 0;

    uint16_t distanciaDerecha = 0;



    // Sensor frontal

    uint16_t distanciaFrontal = 0;



    // Sensor suelo

    uint16_t distanciaSuelo = 0;



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

    ModoSistema modo = ModoSistema::NORMAL;


    bool sistemaSeguro = true;


    bool errorSensor = false;


    bool errorRadar = false;


};



//=====================================================
// ESTADO GLOBAL CENTINELA
//=====================================================

struct EstadoCentinela
{

    EstadoSensores sensores;


    EstadoRadar radar;


    EstadoSalida salida;


    EstadoSistema sistema;


};



// Variable global del sistema

extern EstadoCentinela Estado;



#endif