/**
 * @file    Bateria.h
 * @brief   Módulo de monitoreo de batería (2x 18650 con divisor resistivo) — Centinela v2.0
 */

#ifndef BATERIA_H
#define BATERIA_H

#include <Arduino.h>
#include "Configuracion.h"

class Bateria {
public:
    static void iniciar();
    static void actualizar();
    static uint8_t getPorcentaje();

private:
    Bateria() = delete;
    static uint8_t _porcentaje;
    static int     _lecturaFiltrada;
    static uint8_t _calcularPorcentaje(int lecturaAdc);
};

#endif // BATERIA_H
