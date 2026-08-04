/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Estado.cpp
 * Descripción:
 * Inicialización del estado global.
 **************************************************************************/

#include "Estado.h"



DatosSensores Estado::sensores;

DatosSalida Estado::salida;

DatosSistema Estado::sistema;

DatosApp Estado::app;



void Estado::iniciar()
{

    // Sensores

    sensores.frontal = 400;

    sensores.suelo = 400;

    sensores.radarIzquierda = 400;

    sensores.radarCentro = 400;

    sensores.radarDerecha = 400;


    sensores.inclinacionX = 0;

    sensores.inclinacionY = 0;

    sensores.inclinacionZ = 0;


    sensores.movimientoDetectado = false;

    sensores.caidaDetectada = false;



    // Salidas

    salida.motorIzquierdo = 0;

    salida.motorCentro = 0;

    salida.motorDerecho = 0;

    salida.motorFrontal = 0;


    salida.reproducirAudio = false;

    salida.audioID = 0;



    // Sistema

    sistema.estado = SISTEMA_INICIO;

    sistema.eventoActual = EVENTO_NORMAL;

    sistema.prioridadActual = PRIORIDAD_BAJA;

    sistema.direccion = DIRECCION_NINGUNA;


    sistema.bateria = 0;

    sistema.bluetoothConectado = false;

    sistema.audioOcupado = false;



    // App

    app.navegacionActiva = false;

    app.gpsDisponible = false;

    app.microfonoActivo = false;

    app.recorridoOfflineActivo = false;

}