/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : MPU6050.h
 * Descripción:
 * Módulo de detección de caída mediante MPU6050.
 *
 * Función:
 * - Detectar golpe inicial
 * - Confirmar caída
 * - Enviar estado al sistema
 **************************************************************************/

#ifndef MPU6050_MODULE_H
#define MPU6050_MODULE_H


#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_tockn.h>

#include "Estado.h"



namespace MPU6050_Module
{


//------------------------------------------------------
// Estados internos del detector
//------------------------------------------------------

enum EstadoDeteccion
{
    MPU_NORMAL = 0,

    MPU_GOLPE,

    MPU_CONFIRMACION,

    MPU_CAIDA
};




//------------------------------------------------------
// Funciones públicas
//------------------------------------------------------

void iniciar();

void actualizar();

void reiniciar();




//------------------------------------------------------
// Estado interno accesible
//------------------------------------------------------

EstadoDeteccion obtenerEstado();



}


#endif