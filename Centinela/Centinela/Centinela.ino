 /*
======================================================

                CENTINELA V2

             ARCHIVO PRINCIPAL

              Centinela_V2.ino


  Control general del sistema


======================================================
*/


#include "Configuracion.h"

#include "Estado.h"

#include "Sensores.h"

#include "Radar.h"

#include "Decision.h"

#include "Salida.h"

#include "Comunicacion.h"





//=====================================================
// Estado global del sistema
//=====================================================

EstadoCentinela Estado;





void setup()
{


    // Comunicación

    Comunicacion::iniciar();



    // Sensores

    Sensores::iniciar();



    // Radar

    Radar::iniciar();



    // Decisiones

    Decision::iniciar();



    // Salidas

    Salida::iniciar();



    Serial.println(
        "Sistema listo"
    );


}






void loop()
{


    /*
    Orden de ejecución:

    1) Leer mundo exterior

    2) Actualizar estado

    3) Tomar decisiones

    4) Ejecutar acciones

    5) Comunicación

    */


    Sensores::actualizar();



    Radar::actualizar();



    Decision::actualizar();



    Salida::actualizar();



    Comunicacion::actualizar();



}