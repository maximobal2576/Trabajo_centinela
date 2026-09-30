/**
 * @file    Microfono.h
 * @brief   Módulo de captura de voz ambiental y transmisión por Bluetooth — Centinela v2.0
 */

#ifndef MICROFONO_H
#define MICROFONO_H

#include <Arduino.h>
#include "Configuracion.h"

class Microfono {
public:
    static void iniciar();
    static void actualizar();
    static void iniciarGrabacionManual();
    static void detenerGrabacionManual();
    static bool estaGrabando();

private:
    Microfono() = delete;
    static bool     _grabando;
    static uint32_t _ultimoSonido;
    static uint8_t  _chunkBuffer[256];
    static int      _chunkIndex;

    static void   _enviarChunk();
    static String _base64Encode(const uint8_t* data, size_t length);
};

#endif // MICROFONO_H
