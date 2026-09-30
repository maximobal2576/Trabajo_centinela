/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Microfono.h
 * Descripcion:
 *   Captura de voz del usuario desde el microfono del baston (TRRS sleeve /
 *   modulo analogico o I2S).
 *   Detecta actividad vocal (VAD) o pulsacion de comando, digitaliza audio
 *   PCM 16-bit 16kHz y lo transmite via Bluetooth al celular para
 *   Speech-To-Text y ejecucion de acciones de navegacion y control.
 **************************************************************************/
#ifndef MICROFONO_H
#define MICROFONO_H

#include "Configuracion.h"

class Microfono {
public:
    static void iniciar();
    static void actualizar();

    static void iniciarGrabacionManual();
    static void detenerGrabacionManual();
    static bool estaGrabando();

private:
    static bool          _grabando;
    static unsigned long _ultimoSonido;
    static uint8_t       _chunkBuffer[256];
    static int           _chunkIndex;

    static void _enviarChunk();
    static String _base64Encode(const uint8_t* data, size_t length);
};

#endif // MICROFONO_H
