/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Audio.cpp
 * Descripcion:
 *   Manejo de audio I2S con MAX98357A y conmutacion inteligente de audio:
 *   - Si el jack TRRS detecta auriculares conectados:
 *       -> SD_MODE del MAX98357A se fuerza a LOW (Mute total del altavoz).
 *       -> El altavoz queda 100% en silencio.
 *       -> El audio se canaliza unicamente a los auriculares.
 *   - Si el jack TRRS no tiene auriculares:
 *       -> SD_MODE se activa en HIGH durante la reproduccion para que
 *          el audio suene por el parlante 4 Ohm / 3 W.
 *       -> Al finalizar el audio, SD_MODE vuelve a LOW para ahorrar bateria.
 **************************************************************************/
#include "Audio.h"
#include "Estado.h"
#include "Bluetooth.h"

#include <driver/i2s.h>

bool Audio::_i2sIniciado = false;

// =====================================================
// INICIALIZACION I2S
// =====================================================

void Audio::_initI2S()
{
    i2s_config_t cfg = {
        .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate          = AUDIO_SAMPLE_RATE,
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = I2S_DMA_BUF_COUNT,
        .dma_buf_len          = I2S_DMA_BUF_LEN,
        .use_apll             = false,
        .tx_desc_auto_clear   = true,
        .fixed_mclk           = 0
    };

    i2s_pin_config_t pines = {
        .bck_io_num   = I2S_BCK_PIN,
        .ws_io_num    = I2S_WS_PIN,
        .data_out_num = I2S_DATA_PIN,
        .data_in_num  = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
    i2s_set_pin(I2S_PORT, &pines);
    i2s_zero_dma_buffer(I2S_PORT);

    _i2sIniciado = true;
}

void Audio::iniciar()
{
    // Pin de control SD_MODE del MAX98357A (Shutdown / Mute)
    pinMode(I2S_SD_PIN, OUTPUT);
    digitalWrite(I2S_SD_PIN, LOW);   // Altavoz apagado por defecto

    // Pin de deteccion fisica de jack TRRS
    pinMode(PIN_JACK_DETECT, INPUT_PULLUP);

    _initI2S();

    Serial.println("Audio I2S iniciado (MAX98357A + TRRS Switch)");
}

// =====================================================
// CONSULTA DE AURICULARES
// =====================================================

bool Audio::auricularesConectados()
{
    return (digitalRead(PIN_JACK_DETECT) == NIVEL_JACK_CONECTADO);
}

// =====================================================
// ACTUALIZAR (llamado por Scheduler cada TIEMPO_AUDIO ms)
// =====================================================

void Audio::actualizar()
{
    if (!_i2sIniciado) return;

    // 1. Detectar si hay auriculares conectados en el conector TRRS
    bool hayAuriculares = auricularesConectados();
    Estado::sistema.auricularesConectados = hayAuriculares;

    int disponibles = Bluetooth::audioBytesDisponibles();

    if (disponibles > 0)
    {
        // REGLA CRITICA:
        // Si hay auriculares conectados -> Altavoz apagado (SD_MODE = LOW).
        // Si NO hay auriculares -> Altavoz encendido (SD_MODE = HIGH).
        if (hayAuriculares)
        {
            habilitarAmplificador(false); // Altavoz silenciado al 100%
        }
        else
        {
            habilitarAmplificador(true);  // Altavoz activo
        }

        // Preparar buffer temporal alineado a muestras de 16 bits
        static uint8_t tmpBuf[AUDIO_BUF_SIZE];
        int aLeer = min(disponibles, (int)sizeof(tmpBuf));
        aLeer &= ~1;

        for (int i = 0; i < aLeer; i++)
        {
            tmpBuf[i] = Bluetooth::audioLeer();
        }

        // Escribir muestras al bus I2S (se distribuyen al TRRS y al MAX98357A)
        size_t bytesEscritos = 0;
        i2s_write(I2S_PORT, tmpBuf, aLeer, &bytesEscritos, portMAX_DELAY);
    }
    else
    {
        // Sin reproduccion: apagar amplificador para silenciar siseos y ahorrar energia
        habilitarAmplificador(false);
    }
}

// =====================================================
// CONTROL DEL AMPLIFICADOR DE ALTAVOZ
// =====================================================

void Audio::habilitarAmplificador(bool on)
{
    // Solo se permite encender el altavoz si NO hay auriculares conectados
    if (on && Estado::sistema.auricularesConectados)
    {
        digitalWrite(I2S_SD_PIN, LOW); // Mute forzado
        return;
    }
    digitalWrite(I2S_SD_PIN, on ? HIGH : LOW);
}

void Audio::mute(bool silencio)
{
    if (silencio)
    {
        habilitarAmplificador(false);
        i2s_zero_dma_buffer(I2S_PORT);
    }
}
