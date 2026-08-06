/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Audio.cpp
 * Descripción:
 * Gestión de reproducción de audio.
 **************************************************************************/


#include "Audio.h"

#include "Estado.h"

#include <DFRobotDFPlayerMini.h>



#define AUDIO_SERIAL Serial2



DFRobotDFPlayerMini reproductor;



void Audio::iniciar()
{


    AUDIO_SERIAL.begin(
        AUDIO_BAUDRATE
    );



    if(reproductor.begin(AUDIO_SERIAL))
    {

        Estado::sistema.audioOcupado = false;

    }

    else
    {

        Estado::sistema.audioOcupado = true;

    }


}



void Audio::actualizar()
{

    static unsigned long tiempoUltimoAudio = 0;

    if(Estado::salida.reproducirAudio && !Estado::sistema.audioOcupado)
    {
        reproducir(
            Estado::salida.audioID
        );

        Estado::sistema.audioPendienteID = Estado::salida.audioID;
        Estado::sistema.audioOcupado = true;
        tiempoUltimoAudio = millis();
        Estado::salida.reproducirAudio = false;
    }

    if(Estado::sistema.audioOcupado)
    {
        if(millis() - tiempoUltimoAudio >= AUDIO_TIMEOUT_MS)
        {
            Estado::sistema.audioOcupado = false;
            Estado::sistema.audioPendienteID = 0;
        }
    }

}



void Audio::reproducir(int id)
{


    reproductor.play(id);


}