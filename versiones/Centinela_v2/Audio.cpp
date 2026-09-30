/**
 * @file    Audio.cpp
 * @brief   Implementación del módulo de audio I2S — Centinela v2.0
 */

#include "Audio.h"
#include "Estado.h"
#include "Bluetooth.h"
#include "Decision.h"
#include <esp_arduino_version.h>
#include <math.h>

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    #include <driver/i2s_std.h>
    static i2s_chan_handle_t _i2sTxHandle = nullptr;
#else
    #include <driver/i2s.h>
#endif

bool Audio::_i2sIniciado = false;

void Audio::_initI2S() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    i2s_chan_config_t chanCfg = I2S_CHANNEL_DEFAULT_CONFIG((i2s_port_t)I2S_PORT, I2S_ROLE_MASTER);
    chanCfg.dma_desc_num  = I2S_DMA_BUF_COUNT;
    chanCfg.dma_frame_num = I2S_DMA_BUF_LEN;

    if (i2s_new_channel(&chanCfg, &_i2sTxHandle, nullptr) != ESP_OK) {
        Serial.println(F("  [Audio] ERROR: Fallo al crear canal I2S TX"));
        return;
    }

    i2s_std_config_t stdCfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = (gpio_num_t)I2S_BCK_PIN,
            .ws   = (gpio_num_t)I2S_WS_PIN,
            .dout = (gpio_num_t)I2S_DATA_PIN,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false }
        }
    };

    if (i2s_channel_init_std_mode(_i2sTxHandle, &stdCfg) != ESP_OK) {
        Serial.println(F("  [Audio] ERROR: Fallo al inicializar modo estándar I2S"));
        return;
    }

    if (i2s_channel_enable(_i2sTxHandle) != ESP_OK) {
        Serial.println(F("  [Audio] ERROR: Fallo al habilitar canal I2S"));
        return;
    }
#else
    i2s_config_t cfg = {
        .mode             = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate      = AUDIO_SAMPLE_RATE,
        .bits_per_sample  = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format   = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count    = I2S_DMA_BUF_COUNT,
        .dma_buf_len      = I2S_DMA_BUF_LEN,
        .use_apll         = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk       = 0
    };

    i2s_pin_config_t pines = {
        .bck_io_num   = I2S_BCK_PIN,
        .ws_io_num    = I2S_WS_PIN,
        .data_out_num = I2S_DATA_PIN,
        .data_in_num  = I2S_PIN_NO_CHANGE
    };

    if (i2s_driver_install((i2s_port_t)I2S_PORT, &cfg, 0, NULL) != ESP_OK) {
        Serial.println(F("  [Audio] ERROR: Fallo al instalar driver I2S"));
        return;
    }

    i2s_set_pin((i2s_port_t)I2S_PORT, &pines);
    i2s_zero_dma_buffer((i2s_port_t)I2S_PORT);
#endif

    _i2sIniciado = true;
    Serial.println(F("  [Audio] Driver I2S inicializado (MAX98357A)"));
}

void Audio::iniciar() {
    pinMode(I2S_SD_PIN, OUTPUT);
    digitalWrite(I2S_SD_PIN, LOW); // Amplificador apagado inicialmente

    pinMode(PIN_JACK_DETECT, INPUT_PULLUP);

    _initI2S();
}

bool Audio::auricularesConectados() {
    return (digitalRead(PIN_JACK_DETECT) == NIVEL_JACK_CONECTADO);
}

void Audio::habilitarAmplificador(bool on) {
    if (on && auricularesConectados()) {
        digitalWrite(I2S_SD_PIN, LOW); // Protección: auriculares conectados -> altavoz silenciado
        return;
    }
    digitalWrite(I2S_SD_PIN, on ? HIGH : LOW);
}

void Audio::mute(bool silencio) {
    if (!silencio) return;
    habilitarAmplificador(false);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    if (_i2sTxHandle) {
        static uint8_t zeros[64] = {0};
        size_t written = 0;
        i2s_channel_write(_i2sTxHandle, zeros, sizeof(zeros), &written, 10);
    }
#else
    i2s_zero_dma_buffer((i2s_port_t)I2S_PORT);
#endif
}

void Audio::reproducirTono(uint16_t frecuenciaHz, uint16_t duracionMs) {
    if (!_i2sIniciado) return;

    habilitarAmplificador(true);

    int totalMuestras = (AUDIO_SAMPLE_RATE * duracionMs) / 1000;
    int16_t buffer[64];
    float fase = 0.0f;
    float pasoFase = 2.0f * 3.14159265f * (float)frecuenciaHz / (float)AUDIO_SAMPLE_RATE;

    int muestrasRestantes = totalMuestras;
    while (muestrasRestantes > 0) {
        int bloque = min(muestrasRestantes, 64);
        for (int i = 0; i < bloque; i++) {
            buffer[i] = (int16_t)(sinf(fase) * 16000.0f); // ~50% volumen
            fase += pasoFase;
            if (fase >= 2.0f * 3.14159265f) fase -= 2.0f * 3.14159265f;
        }

        size_t bytesEscritos = 0;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
        i2s_channel_write(_i2sTxHandle, buffer, bloque * sizeof(int16_t), &bytesEscritos, 100);
#else
        i2s_write((i2s_port_t)I2S_PORT, buffer, bloque * sizeof(int16_t), &bytesEscritos, 100);
#endif
        muestrasRestantes -= bloque;
    }

    habilitarAmplificador(false);
}

void Audio::actualizar() {
    if (!_i2sIniciado) return;

    bool hayAuricular = auricularesConectados();
    int disponibles = Bluetooth::audioBytesDisponibles();

    // 1. Si hay streaming de audio por Bluetooth
    if (disponibles > 0) {
        Estado::setAudioReproduciendo(true);
        habilitarAmplificador(!hayAuricular);

        static uint8_t tmpBuf[512];
        int aLeer = min(disponibles, (int)sizeof(tmpBuf));
        aLeer &= ~1;

        for (int i = 0; i < aLeer; i++) {
            tmpBuf[i] = Bluetooth::audioLeer();
        }

        size_t bytesEscritos = 0;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
        i2s_channel_write(_i2sTxHandle, tmpBuf, aLeer, &bytesEscritos, portMAX_DELAY);
#else
        i2s_write((i2s_port_t)I2S_PORT, tmpBuf, aLeer, &bytesEscritos, portMAX_DELAY);
#endif
        return;
    }

    Estado::setAudioReproduciendo(false);

    // 2. Alarma autónoma local si hay emergencia o caída confirmada
    if (Estado::getEmergenciaSOS() || Estado::getCaidaConfirmada()) {
        static uint32_t tSiren = 0;
        if (millis() - tSiren >= 800) {
            tSiren = millis();
            reproducirTono(1200, 150); // Sirena corta de 1200 Hz
        }
        return;
    }

    // 3. Confirmación de avisos de audio desde Decision si corresponde
    if (Decision::solicitaAudio()) {
        Decision::confirmarAudioEmitido();
    }

    habilitarAmplificador(false);
}
