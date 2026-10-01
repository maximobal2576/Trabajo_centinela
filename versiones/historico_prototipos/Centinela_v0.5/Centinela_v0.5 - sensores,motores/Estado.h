/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Estado.h
 * Descripción:
 * Memoria central del sistema.
 **************************************************************************/

#ifndef ESTADO_H
#define ESTADO_H


#include "Tipos.h"


class Estado
{

public:

    // Datos del sistema
    static DatosSensores sensores;

    static DatosSalida salida;

    static DatosSistema sistema;

    static DatosApp app;



    // Inicialización
    static void iniciar();


};



#endif