/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : GPS.cpp
 * Descripcion:
 *   Manejo del puerto Serial2 del ESP32 conectado al GPS NEO-6M.
 *   Parsea sentencias $GPRMC y $GPGGA en tiempo real.
 **************************************************************************/
#include "GPS.h"
#include "Estado.h"

String GPS::_nmeaBuffer = "";

void GPS::iniciar()
{
    // Iniciar Serial2 nativo del ESP32
    Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    _nmeaBuffer.reserve(128);
    Serial.println("Modulo GPS NEO-6M iniciado en Serial2 (GPIO 16/17)");
}

void GPS::actualizar()
{
    while (Serial2.available())
    {
        char c = (char)Serial2.read();
        if (c == '\r') continue;
        if (c == '\n')
        {
            if (_nmeaBuffer.length() > 0)
            {
                _procesarSentencia(_nmeaBuffer);
                _nmeaBuffer = "";
            }
        }
        else
        {
            if (_nmeaBuffer.length() < 120)
            {
                _nmeaBuffer += c;
            }
            else
            {
                _nmeaBuffer = ""; // Overflow protection
            }
        }
    }
}

bool GPS::estaFijado()
{
    return Estado::sistema.gpsFijado;
}

// Convierte formato NMEA (ddmm.mmmm) a grados decimales
float GPS::_convertirCoordenada(const String& valor, char cuadrante)
{
    if (valor.length() < 4) return 0.0f;

    int punto = valor.indexOf('.');
    if (punto < 2) return 0.0f;

    // Los primeros digitos (antes de los ultimos 2 enteros de minutos) son grados
    int gradoLen = punto - 2;
    float grados = valor.substring(0, gradoLen).toFloat();
    float minutos = valor.substring(gradoLen).toFloat();

    float decimal = grados + (minutos / 60.0f);

    if (cuadrante == 'S' || cuadrante == 'W')
    {
        decimal = -decimal;
    }

    return decimal;
}

void GPS::_procesarSentencia(const String& linea)
{
    // Ejemplo $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
    if (linea.startsWith("$GPRMC") || linea.startsWith("$GNRMC"))
    {
        int tokens[13];
        int tokenCount = 0;
        int lastIdx = 0;

        for (int i = 0; i < (int)linea.length() && tokenCount < 13; i++)
        {
            if (linea.charAt(i) == ',')
            {
                tokens[tokenCount++] = i;
            }
        }

        if (tokenCount >= 6)
        {
            // Campo 2: Estado ('A' = Valido / 'V' = No fijado)
            int posEstado = tokens[1] + 1;
            char status = (posEstado < (int)linea.length()) ? linea.charAt(posEstado) : 'V';

            if (status == 'A')
            {
                String rawLat = linea.substring(tokens[2] + 1, tokens[3]);
                char hemLat   = linea.charAt(tokens[3] + 1);

                String rawLon = linea.substring(tokens[4] + 1, tokens[5]);
                char hemLon   = linea.charAt(tokens[5] + 1);

                float lat = _convertirCoordenada(rawLat, hemLat);
                float lon = _convertirCoordenada(rawLon, hemLon);

                if (lat != 0.0f && lon != 0.0f)
                {
                    Estado::sistema.gpsLatitud  = lat;
                    Estado::sistema.gpsLongitud = lon;
                    Estado::sistema.gpsFijado   = true;
                }
            }
            else
            {
                Estado::sistema.gpsFijado = false;
            }
        }
    }
}
