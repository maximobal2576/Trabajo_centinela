/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : GPS.h
 * Descripcion:
 *   Lectura y decodificacion del modulo GPS NEO-6M via Hardware Serial2
 *   (GPIO 16 RX2 / GPIO 17 TX2).
 *   Extrae coordenadas NMEA ($GPRMC / $GPGGA) para geolocalizacion del baston.
 **************************************************************************/
#ifndef GPS_H
#define GPS_H

#include "Configuracion.h"

class GPS {
public:
    static void iniciar();
    static void actualizar();
    static bool estaFijado();

private:
    static String _nmeaBuffer;
    static void _procesarSentencia(const String& linea);
    static float _convertirCoordenada(const String& valor, char cuadrante);
};

#endif // GPS_H
