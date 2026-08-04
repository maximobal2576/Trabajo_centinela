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


    if(
        Estado::salida.reproducirAudio
    )
    {


        reproducir(
            Estado::salida.audioID
        );


        Estado::salida.reproducirAudio = false;

    }


}



void Audio::reproducir(int id)
{


    reproductor.play(id);


}