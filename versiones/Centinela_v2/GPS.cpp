/**
 * @file    GPS.cpp
 * @brief   Implementación del módulo GPS NEO-6M — Centinela v2.0
 *
 * Lee el puerto UART2 por hardware, parsea las sentencias NMEA ($GPRMC/$GNRMC)
 * y actualiza latitud y longitud en Estado.
 *
 * ─── PROTECCIÓN EN CALLE / VIDA REAL ─────────────────────────────────────────
 *  Si el usuario entra en un edificio, túnel o zona techada y se pierde la
 *  señal satelital (Fix = false), las coordenadas NO se borran con ceros.
 *  Se conserva la última posición válida conocida para que una alerta SOS
 *  pueda enviar la ubicación más precisa disponible a los rescatistas.
 * ─────────────────────────────────────────────────────────────────────────────
 */

#include "GPS.h"
#include "Estado.h"

String GPS::_nmeaBuffer = "";

void GPS::iniciar() {
    Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    _nmeaBuffer.reserve(128);
    Serial.println(F("  [GPS] Serial2 inicializado para NEO-6M"));
}

void GPS::actualizar() {
    while (Serial2.available()) {
        char c = (char)Serial2.read();
        if (c == '\r') continue;

        if (c == '\n') {
            if (_nmeaBuffer.length() > 0) {
                _procesarSentencia(_nmeaBuffer);
                _nmeaBuffer = "";
            }
        } else {
            if (_nmeaBuffer.length() < 120) {
                _nmeaBuffer += c;
            } else {
                _nmeaBuffer = "";
            }
        }
    }
}

bool GPS::estaFijado() {
    return Estado::getGPSFix();
}

float GPS::_convertirCoordenada(const String& valor, char cuadrante) {
    if (valor.length() < 4) return 0.0f;

    int punto = valor.indexOf('.');
    if (punto < 2) return 0.0f;

    int gradoLen = punto - 2;
    float grados = valor.substring(0, gradoLen).toFloat();
    float minutos = valor.substring(gradoLen).toFloat();

    float decimal = grados + (minutos / 60.0f);

    if (cuadrante == 'S' || cuadrante == 'W') {
        decimal = -decimal;
    }

    return decimal;
}

void GPS::_procesarSentencia(const String& linea) {
    if (linea.startsWith("$GPRMC") || linea.startsWith("$GNRMC")) {
        int tokens[13];
        int tokenCount = 0;

        for (int i = 0; i < (int)linea.length() && tokenCount < 13; i++) {
            if (linea.charAt(i) == ',') {
                tokens[tokenCount++] = i;
            }
        }

        if (tokenCount >= 6) {
            int posEstado = tokens[1] + 1;
            char status = (posEstado < (int)linea.length()) ? linea.charAt(posEstado) : 'V';

            if (status == 'A') {
                String rawLat = linea.substring(tokens[2] + 1, tokens[3]);
                char hemLat = linea.charAt(tokens[3] + 1);

                String rawLon = linea.substring(tokens[4] + 1, tokens[5]);
                char hemLon = linea.charAt(tokens[5] + 1);

                float lat = _convertirCoordenada(rawLat, hemLat);
                float lon = _convertirCoordenada(rawLon, hemLon);

                if (lat != 0.0f && lon != 0.0f) {
                    Estado::setPosicionGPS(lat, lon);
                    Estado::setGPSFix(true);
                }
            } else {
                // Pérdida de señal: marcar Fix = false, pero NO borrar lat/lon previas
                Estado::setGPSFix(false);
            }
        }
    }
}
