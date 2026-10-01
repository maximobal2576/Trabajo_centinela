#ifndef SALIDA_H
#define SALIDA_H


#include <Arduino.h>


/*
======================================================

                CENTINELA V2

                   SALIDA

  Archivo:
  Salida.h


  Funcion:

  Controla los actuadores físicos:

  - Motores vibradores
  - Audio (futuro)
  - Avisos


======================================================
*/


class Salida
{

public:


    // Inicializa salidas físicas

    static void iniciar();



    // Actualiza actuadores

    static void actualizar();



private:


    static void controlarMotor(
        uint8_t pin,
        uint8_t intensidad
    );


};



#endif