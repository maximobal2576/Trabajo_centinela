/**
 * @file    Bluetooth.cpp
 * @brief   Implementación del módulo Bluetooth SPP — Centinela v2.0
 *
 * 100% Compatible con:
 *   1. App Android Kivy (Centinela_App: EVENTO:SOS, EVENTO:CAIDA, BATERIA, MIC_*, etc.)
 *   2. App Web accesible de Control (presentacion/: ALERTA, ZONA, GPS, etc.)
 */

#include "Bluetooth.h"
#include "Estado.h"
#include "Audio.h"

BluetoothSerial Bluetooth::bt;
uint8_t Bluetooth::audioBuffer[AUDIO_BUF_SIZE];
int Bluetooth::audioHead = 0;
int Bluetooth::audioTail = 0;
String Bluetooth::_cmdBuffer = "";
bool Bluetooth::_esperandoMagic2 = false;
uint32_t Bluetooth::_ultimoHeartbeat = 0;

void Bluetooth::iniciar() {
    bt.setPin(BT_PIN_PAREO);
    bt.begin(BT_NOMBRE_DISPOSITIVO);
    Estado::setBluetoothConectado(false);
    Serial.println(F("  [Bluetooth] SPP iniciado como 'Centinela-Baston'"));
}

void Bluetooth::actualizar() {
    bool conectado = bt.hasClient();
    Estado::setBluetoothConectado(conectado);

    _recibirDatos();

    // Enviar telemetría periódica cada 150 ms si hay conexión
    if (conectado) {
        if (millis() - _ultimoHeartbeat >= TIEMPO_BLUETOOTH) {
            _ultimoHeartbeat = millis();
            enviarEstado();
        }
    }
}

void Bluetooth::enviarComando(const String& cmd) {
    if (bt.hasClient()) {
        bt.println(cmd);
    }
}

void Bluetooth::enviarLinea(const String& linea) {
    if (bt.hasClient()) {
        bt.println(linea);
    }
}

void Bluetooth::enviarDatosRaw(const uint8_t* data, size_t len) {
    if (bt.hasClient() && data && len > 0) {
        bt.write(data, len);
    }
}

void Bluetooth::enviarEstado() {
    PaqueteTelemetria t = Estado::obtenerTelemetria();

    // Compatibilidad App Android (protocolo_baston.py)
    if (Estado::getEmergenciaSOS()) {
        bt.println(F("EVENTO:SOS"));
    } else if (Estado::getCaidaConfirmada()) {
        bt.println(F("EVENTO:CAIDA"));
    } else if (t.nivelAlerta == NivelAlerta::CRITICO) {
        bt.println(F("EVENTO:PELIGRO"));
    } else {
        bt.println(F("EVENTO:NORMAL"));
    }

    bt.print(F("BATERIA:"));   bt.println(t.nivelBateria);

    // Telemetría detallada para App Web
    bt.print(F("ALERTA:"));    bt.println((uint8_t)t.nivelAlerta);
    bt.print(F("ZONA:"));      bt.println((uint8_t)t.zonaPeligro);
    bt.print(F("FRONTAL:"));   bt.println(t.distFrontal);
    bt.print(F("SUELO:"));     bt.println(t.distSuelo);
    bt.print(F("RADAR:"));     bt.println(t.distRadar);
    bt.print(F("IMU:"));       bt.println((uint8_t)t.eventoIMU);
    bt.print(F("SOS:"));       bt.println(Estado::getEmergenciaSOS() ? 1 : 0);
    bt.print(F("CAIDA:"));     bt.println(Estado::getCaidaConfirmada() ? 1 : 0);
    bt.print(F("GPS:"));       bt.print(Estado::getLatitud(), 6); bt.print(F(","));
                               bt.print(Estado::getLongitud(), 6); bt.print(F(","));
                               bt.println(Estado::getGPSFix() ? 1 : 0);
}

void Bluetooth::_recibirDatos() {
    while (bt.available()) {
        int raw = bt.read();
        if (raw < 0) break;
        uint8_t b = (uint8_t)raw;

        // Detección de cabecera mágica (Audio PCM entrante desde Android o Web)
        if (!_esperandoMagic2 && b == BT_AUDIO_MAGIC_1) {
            _esperandoMagic2 = true;
            continue;
        }

        if (_esperandoMagic2) {
            _esperandoMagic2 = false;

            if (b == BT_AUDIO_MAGIC_2) {
                // Trama binaria de audio detectada
                unsigned long t0 = millis();
                while (bt.available() < 2 && millis() - t0 < 20) {}
                if (bt.available() < 2) continue;

                uint8_t lo = (uint8_t)bt.read();
                uint8_t hi = (uint8_t)bt.read();
                int chunkLen = (int)lo | ((int)hi << 8);
                chunkLen = constrain(chunkLen, 0, AUDIO_BUF_SIZE / 2);

                int recibidos = 0;
                t0 = millis();
                while (recibidos < chunkLen && millis() - t0 < 80) {
                    if (bt.available()) {
                        uint8_t dato = (uint8_t)bt.read();
                        int siguiente = (audioHead + 1) % AUDIO_BUF_SIZE;
                        if (siguiente != audioTail) {
                            audioBuffer[audioHead] = dato;
                            audioHead = siguiente;
                        }
                        recibidos++;
                    }
                }
                continue;
            } else {
                _cmdBuffer += (char)BT_AUDIO_MAGIC_1;
            }
        }

        // Recepción de comandos de texto
        if (b == '\n' || b == '\r') {
            _cmdBuffer.trim();
            if (_cmdBuffer.length() > 0) {
                _procesarComando(_cmdBuffer);
            }
            _cmdBuffer = "";
        } else if (_cmdBuffer.length() < 120) {
            _cmdBuffer += (char)b;
        }
    }
}

int Bluetooth::audioBytesDisponibles() {
    int disp = audioHead - audioTail;
    if (disp < 0) disp += AUDIO_BUF_SIZE;
    return disp;
}

uint8_t Bluetooth::audioLeer() {
    if (audioTail == audioHead) return 0;
    uint8_t b = audioBuffer[audioTail];
    audioTail = (audioTail + 1) % AUDIO_BUF_SIZE;
    return b;
}

void Bluetooth::_procesarComando(const String& cmd) {
    if (cmd == "PING") {
        bt.println(F("PONG"));
    } else if (cmd == "SOS:1") {
        Estado::setEmergenciaSOS(true);
    } else if (cmd == "SOS:0") {
        Estado::setEmergenciaSOS(false);
    } else if (cmd == "B:1") {
        // Baliza acústica de esquina solicitada por la app Android
        Audio::reproducirTono(880, 200);
    } else if (cmd == "B:0") {
        Audio::mute(true);
    } else if (cmd.startsWith("TEST:")) {
        Serial.print(F("Test comando: ")); Serial.println(cmd);
    }
}
