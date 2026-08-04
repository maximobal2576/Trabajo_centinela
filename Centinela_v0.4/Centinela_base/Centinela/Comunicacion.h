#ifndef COMUNICACION_H
#define COMUNICACION_H


#include <Arduino.h>



/*
======================================================

                CENTINELA V2

              COMUNICACION

  Archivo:
  Comunicacion.h


  Funciones:

  - Monitor serie
  - Bluetooth (futuro)
  - DFPlayer (futuro)
  - Diagnostico
  - Configuracion externa


======================================================
*/


class Comunicacion
{

public:


    // Inicializar comunicaciones

    static void iniciar();



    // Actualizar comunicaciones

    static void actualizar();



private:


    static void enviarEstado();


    static void recibirComando();



};



#endif