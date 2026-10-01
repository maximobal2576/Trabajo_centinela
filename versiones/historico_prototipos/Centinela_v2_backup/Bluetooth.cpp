/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Bluetooth.cpp
 * Descripcion:
 *   BT SPP con BluetoothSerial del ESP32.
 *   Protocolo dual:
 *   - Comandos de texto y telemetria (\n)
 *   - Stream de audio PCM entrante hacia MAX98357A / Auriculares
 *   - Transmision de telemetria de sensores, GPS, jack TRRS y microfono
 **************************************************************************/
#include "Bluetooth.h"
#include "Estado.h"

BluetoothSerial Bluetooth::bt;

uint8_t Bluetooth::audioBuffer[AUDIO_BUF_SIZE];
int     Bluetooth::audioHead = 0;
int     Bluetooth::audioTail = 0;
String  Bluetooth::_cmdBuffer = "";
bool    Bluetooth::_esperandoMagic2 = false;

// =====================================================
// INICIAR
// =====================================================

void Bluetooth::iniciar()
{
    bt.setPin(BT_PIN_PAREO);
    bt.begin(BT_NOMBRE_DISPOSITIVO);
    Estado::sistema.bluetoothConectado = false;

    Serial.println("BT SPP iniciado como: " BT_NOMBRE_DISPOSITIVO);
    Serial.println("PIN de pareo: " BT_PIN_PAREO);
}

// =====================================================
// ACTUALIZAR (llamado por Scheduler cada 150 ms)
// =====================================================

void Bluetooth::actualizar()
{
    recibirDatos();
    if (Estado::sistema.bluetoothConectado)
    {
        enviarEstado();
    }
}

// =====================================================
// ENVIAR COMANDOS Y LINEAS
// =====================================================

void Bluetooth::enviarComando(const String& cmd)
{
    if (Estado::sistema.bluetoothConectado)
    {
        bt.println(cmd);
    }
}

void Bluetooth::enviarLinea(const String& linea)
{
    if (Estado::sistema.bluetoothConectado)
    {
        bt.println(linea);
    }
}

void Bluetooth::enviarDatosRaw(const uint8_t* data, size_t len)
{
    if (Estado::sistema.bluetoothConectado && data && len > 0)
    {
        bt.write(data, len);
    }
}

// =====================================================
// ENVIAR ESTADO A LA APP
// =====================================================

void Bluetooth::enviarEstado()
{
    bt.print("EVENTO:");
    bt.println(Estado::sistema.eventoActual);

    bt.print("DIRECCION:");
    bt.println(Estado::sistema.direccion);

    bt.print("BATERIA:");
    bt.println(Estado::sistema.bateria);

    bt.print("AUDIO:");
    bt.println(Estado::sistema.audioStreamActivo ? 1 : 0);

    bt.print("BLUETOOTH:");
    bt.println(Estado::sistema.bluetoothConectado ? 1 : 0);

    bt.print("ESTADO:");
    bt.println(Estado::sistema.estado);

    // Estado del jack de auriculares TRRS
    bt.print("AURICULAR:");
    bt.println(Estado::sistema.auricularesConectados ? 1 : 0);

    // Telemetria GPS
    bt.print("GPS:");
    bt.print(Estado::sistema.gpsLatitud, 6);
    bt.print(",");
    bt.print(Estado::sistema.gpsLongitud, 6);
    bt.print(",");
    bt.println(Estado::sistema.gpsFijado ? 1 : 0);
}

// =====================================================
// RECIBIR DATOS (comandos de texto + chunks de audio)
// =====================================================

void Bluetooth::recibirDatos()
{
    while (bt.available())
    {
        Estado::sistema.bluetoothConectado = true;

        int raw = bt.read();
        if (raw < 0) break;
        uint8_t b = (uint8_t)raw;

        // Detector de magic header de audio entrante (Downlink 0xA5 0x5A)
        if (!_esperandoMagic2 && b == BT_AUDIO_MAGIC_1)
        {
            _esperandoMagic2 = true;
            continue;
        }

        if (_esperandoMagic2)
        {
            _esperandoMagic2 = false;
            if (b == BT_AUDIO_MAGIC_2)
            {
                unsigned long t0 = millis();
                while (bt.available() < 2 && millis() - t0 < 20) { }
                if (bt.available() < 2) continue;

                uint8_t lo = (uint8_t)bt.read();
                uint8_t hi = (uint8_t)bt.read();
                int chunkLen = (int)lo | ((int)hi << 8);
                chunkLen = constrain(chunkLen, 0, AUDIO_BUF_SIZE / 2);

                int recibidos = 0;
                t0 = millis();
                while (recibidos < chunkLen && millis() - t0 < 100)
                {
                    if (bt.available())
                    {
                        uint8_t dato = (uint8_t)bt.read();
                        int siguiente = (audioHead + 1) % AUDIO_BUF_SIZE;
                        if (siguiente != audioTail)
                        {
                            audioBuffer[audioHead] = dato;
                            audioHead = siguiente;
                        }
                        recibidos++;
                    }
                }

                Estado::sistema.audioStreamActivo  = true;
                Estado::sistema.ultimoChunkAudio   = millis();
                Estado::salida.reproducirAudio     = true;
                continue;
            }
            else
            {
                _cmdBuffer += (char)BT_AUDIO_MAGIC_1;
            }
        }

        // Parser de comandos de texto
        if (b == '\n' || b == '\r')
        {
            _cmdBuffer.trim();
            if (_cmdBuffer.length() > 0)
            {
                _procesarComando(_cmdBuffer);
            }
            _cmdBuffer = "";
        }
        else if (_cmdBuffer.length() < 250)
        {
            _cmdBuffer += (char)b;
        }
    }

    if (Estado::sistema.audioStreamActivo &&
        millis() - Estado::sistema.ultimoChunkAudio > AUDIO_STREAM_TIMEOUT_MS)
    {
        Estado::sistema.audioStreamActivo = false;
        Estado::salida.reproducirAudio    = false;
    }
}

// =====================================================
// BUFFER CIRCULAR DE AUDIO PCM
// =====================================================

int Bluetooth::audioBytesDisponibles()
{
    int disp = audioHead - audioTail;
    if (disp < 0) disp += AUDIO_BUF_SIZE;
    return disp;
}

uint8_t Bluetooth::audioLeer()
{
    if (audioTail == audioHead) return 0;
    uint8_t b = audioBuffer[audioTail];
    audioTail = (audioTail + 1) % AUDIO_BUF_SIZE;
    return b;
}

// =====================================================
// PROCESAR COMANDOS DE TEXTO
// =====================================================

void Bluetooth::_procesarComando(const String& cmd)
{
    if (cmd == "PING")
    {
        bt.println("PONG");
    }
    else if (cmd.startsWith("TEST:"))
    {
        int nivel = cmd.substring(5).toInt();
        Estado::salida.motorIzquierdo = nivel;
        Estado::salida.motorCentro    = nivel;
        Estado::salida.motorDerecho   = nivel;
    }
    else if (cmd.startsWith("N:"))
    {
        Estado::app.navegacionActiva = (cmd.charAt(2) == '1');
    }
    else if (cmd.startsWith("G:"))
    {
        Estado::app.gpsDisponible = (cmd.charAt(2) == '1');
    }
    else if (cmd.startsWith("B:"))
    {
        // Baliza acustica de llegada
    }
    else if (cmd.startsWith("I:"))
    {
        // Texto de navegacion recibido
        Serial.print("Indicacion de app: ");
        Serial.println(cmd.substring(2));
    }
}
