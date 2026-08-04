/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Scheduler.cpp
 **************************************************************************/


#include "Scheduler.h"


#include "Sensores.h"
#include "Radar.h"

#include "Decision.h"
#include "Salida.h"
#include "Bluetooth.h"
#include "Audio.h"
#include "Bateria.h"
#include "Diagnostico.h"

#include "MPU6050.h"





unsigned long Scheduler::tiempoSensores = 0;

unsigned long Scheduler::tiempoRadar = 0;

unsigned long Scheduler::tiempoMPU = 0;



unsigned long Scheduler::tiempoDecision = 0;

unsigned long Scheduler::tiempoSalida = 0;

unsigned long Scheduler::tiempoBluetooth = 0;

unsigned long Scheduler::tiempoAudio = 0;

unsigned long Scheduler::tiempoBateria = 0;







void Scheduler::iniciar()
{

    unsigned long ahora = millis();



    tiempoSensores = ahora;

    tiempoRadar = ahora;

    tiempoMPU = ahora;


    tiempoDecision = ahora;

    tiempoSalida = ahora;

    tiempoBluetooth = ahora;

    tiempoAudio = ahora;

    tiempoBateria = ahora;


}









void Scheduler::actualizar()
{


    unsigned long ahora = millis();





    //--------------------------------------------------
    // Sensores ultrasónicos
    //--------------------------------------------------

    if(ahora - tiempoSensores >= 50)
    {

        tiempoSensores = ahora;

        Sensores::actualizar();

    }







    //--------------------------------------------------
    // Radar
    //--------------------------------------------------

    if(ahora - tiempoRadar >= 70)
    {

        tiempoRadar = ahora;

        Radar::actualizar();

    }








    //--------------------------------------------------
    // MPU6050
    //--------------------------------------------------

    if(ahora - tiempoMPU >= 20)
    {

        tiempoMPU = ahora;

        MPU6050_Module::actualizar();

    }














    //--------------------------------------------------
    // Decisiones
    //--------------------------------------------------

    if(ahora - tiempoDecision >= 50)
    {

        tiempoDecision = ahora;

        Decision::actualizar();

    }








    //--------------------------------------------------
    // Salidas
    //--------------------------------------------------

    if(ahora - tiempoSalida >= 20)
    {

        tiempoSalida = ahora;

        Salida::actualizar();

    }








    //--------------------------------------------------
    // Bluetooth
    //--------------------------------------------------

    if(ahora - tiempoBluetooth >= 200)
    {

        tiempoBluetooth = ahora;

        Bluetooth::actualizar();

    }








    //--------------------------------------------------
    // Audio
    //--------------------------------------------------

    if(ahora - tiempoAudio >= 50)
    {

        tiempoAudio = ahora;

        Audio::actualizar();

    }








    //--------------------------------------------------
    // Batería
    //--------------------------------------------------

    if(ahora - tiempoBateria >= 1000)
    {

        tiempoBateria = ahora;

        Bateria::actualizar();

    }




}