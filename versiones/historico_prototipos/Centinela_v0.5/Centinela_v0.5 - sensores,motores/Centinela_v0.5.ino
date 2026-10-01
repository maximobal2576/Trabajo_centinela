/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Centinela_v1.ino
 **************************************************************************/


#include "Configuracion.h"

#include "Tipos.h"

#include "Estado.h"

#include "Scheduler.h"

#include "Sensores.h"
#include "Radar.h"
#include "MPU6050.h"
#include "Decision.h"
#include "Salida.h"
#include "Bluetooth.h"
#include "Audio.h"
#include "Bateria.h"
#include "Diagnostico.h"



void setup()
{

    Serial.begin(9600);


    Serial.println("====================");
    Serial.println("CENTINELA v1.0");
    Serial.println("Iniciando sistema...");
    Serial.println("====================");



    Estado::iniciar();



    Sensores::iniciar();

    Radar::iniciar();

    MPU6050_Module::iniciar();

    Decision::iniciar();

    Salida::iniciar();

    Bluetooth::iniciar();

    Audio::iniciar();

    Bateria::iniciar();

    Diagnostico::iniciar();



    Scheduler::iniciar();



    Estado::sistema.estado = SISTEMA_NORMAL;



    Serial.println("Sistema listo");

}



void loop()
{

    Scheduler::actualizar();

}