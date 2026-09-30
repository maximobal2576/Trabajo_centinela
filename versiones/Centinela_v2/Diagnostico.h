/**
 * @file    Diagnostico.h
 * @brief   Módulo de telemetría y diagnóstico por consola USB — Centinela v2.0
 */

#ifndef DIAGNOSTICO_H
#define DIAGNOSTICO_H

#include <Arduino.h>
#include "Configuracion.h"

class Diagnostico {
public:
    static void iniciar();
    static void actualizar();

private:
    Diagnostico() = delete;
};

#endif // DIAGNOSTICO_H
