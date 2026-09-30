/**
 * @file    Audio.h
 * @brief   Módulo de audio I2S (MAX98357A + Jack TRRS con conmutación automática) — Centinela v2.0
 *
 * Funcionalidades clave en la calle:
 *   1. Streaming PCM desde la app vía Bluetooth para guía de voz o comandos.
 *   2. Generador acústico de emergencia autónomo: Si hay caída o SOS y el teléfono
 *      está desconectado, el bastón emite por sí mismo una sirena de auxilio.
 *   3. Conmutación inteligente TRRS: Si se conectan auriculares (GPIO 14 = LOW),
 *      se apaga el parlante externo (I2S_SD_PIN = LOW) para privacidad.
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "Configuracion.h"

class Audio {
public:
    static void iniciar();
    static void actualizar();
    static void habilitarAmplificador(bool on);
    static bool auricularesConectados();
    static void mute(bool silencio);

    /**
     * @brief Emite un tono sintético autónomo de alerta sin depender de Bluetooth.
     * @param frecuenciaHz Frecuencia del tono (ej. 880 Hz o 1200 Hz).
     * @param duracionMs   Duración en milisegundos.
     */
    static void reproducirTono(uint16_t frecuenciaHz, uint16_t duracionMs);

private:
    Audio() = delete;
    static bool _i2sIniciado;
    static void _initI2S();
};

#endif // AUDIO_H
