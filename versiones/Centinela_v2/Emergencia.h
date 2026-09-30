/**
 * @file    Emergencia.h
 * @brief   Módulo de gestión del botón físico SOS de emergencia — Centinela v2.0
 */

#ifndef EMERGENCIA_H
#define EMERGENCIA_H

#include <Arduino.h>
#include "Configuracion.h"

class Emergencia {
public:
    static void iniciar();
    static void actualizar();
    static bool botonPresionado();

private:
    Emergencia() = delete;
    static uint32_t _ultimoCambio;
    static int      _ultimoEstadoFisico;
    static bool     _estadoConfirmado;
};

#endif // EMERGENCIA_H
