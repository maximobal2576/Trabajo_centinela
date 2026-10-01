/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Microfono.cpp
 * Descripcion:
 *   Digitalizacion y transmision de voz desde el baston hacia la app.
 *   Utiliza el pin analogico del microfono (TRRS Sleeve o modulo electret)
 *   o ADC1. Al detectar voz (VAD con umbral) envia:
 *     MIC_START
 *     MIC_CHUNK:<base64>
 *     ...
 *     MIC_STOP
 **************************************************************************/
#include "Microfono.h"
#include "Estado.h"
#include "Bluetooth.h"

bool          Microfono::_grabando      = false;
unsigned long Microfono::_ultimoSonido  = 0;
uint8_t       Microfono::_chunkBuffer[256];
int           Microfono::_chunkIndex    = 0;

static const char b64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

String Microfono::_base64Encode(const uint8_t* data, size_t length)
{
    String result = "";
    result.reserve(((length + 2) / 3) * 4);

    size_t i = 0;
    while (i < length)
    {
        uint32_t octet_a = (i < length) ? data[i++] : 0;
        uint32_t octet_b = (i < length) ? data[i++] : 0;
        uint32_t octet_c = (i < length) ? data[i++] : 0;

        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        result += b64_chars[(triple >> 18) & 0x3F];
        result += b64_chars[(triple >> 12) & 0x3F];
        result += (i > length + 1) ? '=' : b64_chars[(triple >> 6) & 0x3F];
        result += (i > length) ? '=' : b64_chars[triple & 0x3F];
    }
    return result;
}

void Microfono::iniciar()
{
    pinMode(PIN_MICROFONO_ANALOGICO, INPUT);
    _grabando = false;
    _chunkIndex = 0;
    _ultimoSonido = 0;
    Serial.println("Microfono del baston iniciado en GPIO 39 (ADC1)");
}

bool Microfono::estaGrabando()
{
    return _grabando;
}

void Microfono::iniciarGrabacionManual()
{
    if (!_grabando)
    {
        _grabando = true;
        _chunkIndex = 0;
        _ultimoSonido = millis();
        Bluetooth::enviarLinea("MIC_START");
        Serial.println("[Microfono] Grabacion iniciada...");
    }
}

void Microfono::detenerGrabacionManual()
{
    if (_grabando)
    {
        if (_chunkIndex > 0)
        {
            _enviarChunk();
        }
        _grabando = false;
        Bluetooth::enviarLinea("MIC_STOP");
        Serial.println("[Microfono] Grabacion finalizada. Enviado a la app.");
    }
}

void Microfono::_enviarChunk()
{
    if (_chunkIndex == 0) return;
    String b64 = _base64Encode(_chunkBuffer, _chunkIndex);
    Bluetooth::enviarLinea("MIC_CHUNK:" + b64);
    _chunkIndex = 0;
}

void Microfono::actualizar()
{
    if (!Estado::sistema.bluetoothConectado) return;

    // Tomar una muestra del microfono (0 a 4095)
    // Centrado de audio alrededor de 2048 (offset DC)
    int muestra = analogRead(PIN_MICROFONO_ANALOGICO);
    int amplitud = abs(muestra - 2048);

    // Deteccion de voz por umbral (VAD)
    if (amplitud > MIC_UMBRAL_VOZ)
    {
        _ultimoSonido = millis();
        if (!_grabando)
        {
            iniciarGrabacionManual();
        }
    }

    if (_grabando)
    {
        // Convertir a PCM signed 16-bit
        int16_t pcmSample = (int16_t)((muestra - 2048) * 16);

        _chunkBuffer[_chunkIndex++] = (uint8_t)(pcmSample & 0xFF);
        _chunkBuffer[_chunkIndex++] = (uint8_t)((pcmSample >> 8) & 0xFF);

        if (_chunkIndex >= (int)sizeof(_chunkBuffer))
        {
            _enviarChunk();
        }

        // Si pasan 700 ms de silencio, detener grabacion y despachar a la app
        if (millis() - _ultimoSonido > 700)
        {
            detenerGrabacionManual();
        }
    }
}
