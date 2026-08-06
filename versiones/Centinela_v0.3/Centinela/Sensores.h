#ifndef SENSORES_H
#define SENSORES_H


#include <Arduino.h>


/*
======================================================

                CENTINELA V2

                  SENSORES

  Archivo:
  Sensores.h


  Funcion:

  Interfaz del modulo encargado de leer:

  - Sensor frontal
  - Sensor suelo
  - Ultrasonico radar
  - MPU6050 (futuro)
  - Bateria (futuro)


======================================================
*/



class Sensores
{

public:


    // Inicialización de sensores

    static void iniciar();



    // Actualización periódica

    static void actualizar();



    // Lecturas individuales

    static uint16_t leerDistancia(
        uint8_t trig,
        uint8_t echo
    );



private:


    // Promedio de lecturas ultrasónicas

    static uint16_t filtrarLectura(
        uint8_t trig,
        uint8_t echo
    );


};


#endif