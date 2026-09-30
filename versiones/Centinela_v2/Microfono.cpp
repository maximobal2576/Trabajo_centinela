/**
 * @file    Microfono.cpp
 * @brief   Implementación del micrófono electret con VAD y codificación Base64 — Centinela v2.0
 */

#include "Microfono.h"
#include "Estado.h"
#include "Bluetooth.h"

bool     Microfono::_grabando = false;
uint32_t Microfono::_ultimoSonido = 0;
uint8_t  Microfono::_chunkBuffer[256];
int      Microfono::_chunkIndex = 0;

static const char b64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

String Microfono::_base64Encode(const uint8_t* data, size_t length) {
    String result = "";
    result.reserve(((length + 2) / 3) * 4);

    size_t i = 0;
    while (i < length) {
        uint32_t a = (i < length) ? data[i++] : 0;
        uint32_t b = (i < length) ? data[i++] : 0;
        uint32_t c = (i < length) ? data[i++] : 0;

        uint32_t triple = (a << 16) + (b << 8) + c;

        result += b64_chars[(triple >> 18) & 0x3F];
        result += b64_chars[(triple >> 12) & 0x3F];
        result += (i > length + 1) ? '=' : b64_chars[(triple >> 6) & 0x3F];
        result += (i > length)     ? '=' : b64_chars[triple & 0x3F];
    }
    return result;
}

void Microfono::iniciar() {
    pinMode(PIN_MICROFONO, INPUT);
    _grabando = false;
    _chunkIndex = 0;
    _ultimoSonido = 0;
    Serial.println(F("  [Microfono] Entrada analógica en GPIO 39"));
}

bool Microfono::estaGrabando() {
    return _grabando;
}

void Microfono::iniciarGrabacionManual() {
    if (!_grabando) {
        _grabando = true;
        _chunkIndex = 0;
        _ultimoSonido = millis();
        Bluetooth::enviarLinea(F("MIC_START"));
    }
}

void Microfono::detenerGrabacionManual() {
    if (_grabando) {
        if (_chunkIndex > 0) {
            _enviarChunk();
        }
        _grabando = false;
        Bluetooth::enviarLinea(F("MIC_STOP"));
    }
}

void Microfono::_enviarChunk() {
    if (_chunkIndex == 0) return;
    String b64 = _base64Encode(_chunkBuffer, _chunkIndex);
    Bluetooth::enviarLinea("MIC_CHUNK:" + b64);
    _chunkIndex = 0;
}

void Microfono::actualizar() {
    if (!Estado::getBluetoothConectado()) return;

    int muestra = analogRead(PIN_MICROFONO);
    int amplitud = abs(muestra - 2048);

    if (amplitud > MIC_UMBRAL_VOZ) {
        _ultimoSonido = millis();
        if (!_grabando) {
            iniciarGrabacionManual();
        }
    }

    if (_grabando) {
        int16_t pcmSample = (int16_t)((muestra - 2048) * 16);

        _chunkBuffer[_chunkIndex++] = (uint8_t)(pcmSample & 0xFF);
        _chunkBuffer[_chunkIndex++] = (uint8_t)((pcmSample >> 8) & 0xFF);

        if (_chunkIndex >= (int)sizeof(_chunkBuffer)) {
            _enviarChunk();
        }

        if (millis() - _ultimoSonido > MIC_SILENCIO_TIMEOUT_MS) {
            detenerGrabacionManual();
        }
    }
}
