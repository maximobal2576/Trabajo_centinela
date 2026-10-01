/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Bluetooth.h
 * Descripción:
 * Comunicación Bluetooth HC-05.
 **************************************************************************/

#ifndef BLUETOOTH_H
#define BLUETOOTH_H


#include <Arduino.h>

#include "Configuracion.h"



class Bluetooth
{

public:

    static void iniciar();

    static void actualizar();



private:

    static void enviarEstado();

    static void recibirComandos();



};



#endif