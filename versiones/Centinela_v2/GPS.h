/**
 * @file    GPS.h
 * @brief   Módulo de geolocalización NMEA (NEO-6M por Serial2) — Centinela v2.0
 */

#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include "Configuracion.h"

class GPS {
public:
    static void iniciar();
    static void actualizar();
    static bool estaFijado();

private:
    GPS() = delete;
    static String _nmeaBuffer;
    static void _procesarSentencia(const String& linea);
    static float _convertirCoordenada(const String& valor, char cuadrante);
};

#endif // GPS_H
