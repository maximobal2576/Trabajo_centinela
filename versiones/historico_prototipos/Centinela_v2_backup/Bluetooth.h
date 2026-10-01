/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Bluetooth.h
 * Descripcion:
 *   Comunicacion Bluetooth SPP via BluetoothSerial integrado del ESP32.
 *   El ESP32 actua como servidor: siempre anunciado y listo para conectar.
 *   Recibe comandos de texto y stream PCM hacia el altavoz/auricular.
 *   Transmite telemetria de sensores, GPS, jack TRRS y stream de microfono.
 **************************************************************************/
#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BluetoothSerial.h>
#include "Configuracion.h"

// Prefijo que identifica un paquete de audio PCM entrante (Downlink)
#define BT_AUDIO_MAGIC_1  0xA5
#define BT_AUDIO_MAGIC_2  0x5A

class Bluetooth {
public:
    static void iniciar();
    static void actualizar();

    // Envia estado completo del baston a la app
    static void enviarEstado();

    // Envia linea de comando o texto por SPP
    static void enviarComando(const String& cmd);
    static void enviarLinea(const String& linea);
    static void enviarDatosRaw(const uint8_t* data, size_t len);

    // Lee comandos y datos del socket BT
    static void recibirDatos();

    // Buffer circular de audio PCM recibido por BT
    static uint8_t  audioBuffer[AUDIO_BUF_SIZE];
    static int      audioHead;   // indice de escritura
    static int      audioTail;   // indice de lectura
    static int      audioBytesDisponibles();
    static uint8_t  audioLeer();

private:
    static BluetoothSerial bt;
    static String  _cmdBuffer;
    static void    _procesarComando(const String& cmd);
    static bool    _esperandoMagic2;
};

#endif // BLUETOOTH_H
