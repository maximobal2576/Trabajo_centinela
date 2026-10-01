#ifndef DECISION_H
#define DECISION_H


#include <Arduino.h>


/*
======================================================

                CENTINELA V2

                  DECISION

  Archivo:
  Decision.h


  Funcion:

  Modulo encargado de interpretar
  el estado del sistema y decidir
  acciones.


======================================================
*/



class Decision
{

public:


    // Inicialización

    static void iniciar();



    // Analiza todos los sistemas

    static void actualizar();



private:


    // Análisis individual


    static void analizarRadar();


    static void analizarFrontal();


    static void analizarSuelo();


    static void analizarCaida();



    // Gestión de modos


    static void gestionarModo();



};



#endif