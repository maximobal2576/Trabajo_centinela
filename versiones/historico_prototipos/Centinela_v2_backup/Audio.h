/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Audio.h
 * Descripcion:
 *   Gestion del decodificador I2S (MAX98357A) y TRRS Breakout.
 *   Al conectar los auriculares al jack 3.5mm TRRS, el altavoz se
 *   apaga completamente (SD_MODE = LOW / Mute total) y el audio suena
 *   exclusivamente por los auriculares.
 **************************************************************************/
#ifndef AUDIO_H
#define AUDIO_H

#include "Configuracion.h"

class Audio {
public:
    static void iniciar();

    // Llamado periodicamente por Scheduler cada TIEMPO_AUDIO ms.
    // Gestiona deteccion de jack, conmutacion altavoz/auricular y stream I2S.
    static void actualizar();

    // Control directo del pin SD_MODE del MAX98357A
    static void habilitarAmplificador(bool on);

    // Consulta si los auriculares estan insertados en el TRRS
    static bool auricularesConectados();

    // Silencio inmediato de buffer DMA
    static void mute(bool silencio);

private:
    static bool _i2sIniciado;
    static void _initI2S();
};

#endif // AUDIO_H
