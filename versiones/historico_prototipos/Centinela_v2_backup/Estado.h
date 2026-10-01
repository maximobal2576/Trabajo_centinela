/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Estado.h
 **************************************************************************/
#ifndef ESTADO_H
#define ESTADO_H

#include "Tipos.h"

class Estado {
public:
    static DatosSensores sensores;
    static DatosSalida   salida;
    static DatosSistema  sistema;
    static DatosApp      app;

    static void iniciar();
};

#endif // ESTADO_H
