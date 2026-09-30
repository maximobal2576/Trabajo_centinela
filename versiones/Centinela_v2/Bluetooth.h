/**
 * @file    Bluetooth.h
 * @brief   Módulo de comunicación Bluetooth SPP dual (Comandos + Audio PCM) — Centinela v2.0
 */

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include <BluetoothSerial.h>
#include "Configuracion.h"
#include "Tipos.h"

#define BT_AUDIO_MAGIC_1  0xA5
#define BT_AUDIO_MAGIC_2  0x5A

class Bluetooth {
public:
    static void iniciar();
    static void actualizar();

    static void enviarEstado();
    static void enviarComando(const String& cmd);
    static void enviarLinea(const String& linea);
    static void enviarDatosRaw(const uint8_t* data, size_t len);

    static int audioBytesDisponibles();
    static uint8_t audioLeer();

    static uint8_t audioBuffer[AUDIO_BUF_SIZE];
    static int audioHead;
    static int audioTail;

private:
    Bluetooth() = delete;
    static BluetoothSerial bt;
    static String _cmdBuffer;
    static bool _esperandoMagic2;
    static uint32_t _ultimoHeartbeat;

    static void _recibirDatos();
    static void _procesarComando(const String& cmd);
};

#endif // BLUETOOTH_H
