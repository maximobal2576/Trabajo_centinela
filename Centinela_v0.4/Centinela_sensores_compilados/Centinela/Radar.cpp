#ifndef RADAR_CPP
#define RADAR_CPP


#include <Servo.h>

#include "Radar.h"
#include "Configuracion.h"
#include "Estado.h"
#include "Sensores.h"



Servo servoRadar;



Radar::PasoRadar Radar::pasoActual =
Radar::PasoRadar::IR_IZQUIERDA;



unsigned long Radar::tiempoAnterior = 0;



void Radar::iniciar()
{


    servoRadar.attach(
        PIN_SERVO_RADAR
    );


    mover(
        RADAR_CENTRO
    );


    Estado.radar.activo = true;


}






void Radar::actualizar()
{


    unsigned long ahora =
        millis();



    if(
        ahora - tiempoAnterior
        <
        TIEMPO_LECTURA_RADAR
    )
    {

        return;

    }



    tiempoAnterior = ahora;



    switch(pasoActual)
    {


        case PasoRadar::IR_IZQUIERDA:


            mover(
                RADAR_IZQUIERDA_EXTREMA
            );


            pasoActual =
                PasoRadar::LEER_IZQUIERDA;


        break;





        case PasoRadar::LEER_IZQUIERDA:


            Estado.sensores.radar30 =
                Sensores::leerDistancia(
                    TRIG_RADAR,
                    ECHO_RADAR
                );



            pasoActual =
                PasoRadar::IR_CENTRO;


        break;





        case PasoRadar::IR_CENTRO:


            mover(
                RADAR_CENTRO
            );


            pasoActual =
                PasoRadar::LEER_CENTRO;


        break;





        case PasoRadar::LEER_CENTRO:


            Estado.sensores.radar90 =
                Sensores::leerDistancia(
                    TRIG_RADAR,
                    ECHO_RADAR
                );


            pasoActual =
                PasoRadar::IR_DERECHA;


        break;





        case PasoRadar::IR_DERECHA:


            mover(
                RADAR_DERECHA_EXTREMA
            );


            pasoActual =
                PasoRadar::LEER_DERECHA;


        break;





        case PasoRadar::LEER_DERECHA:


            Estado.sensores.radar150 =
                Sensores::leerDistancia(
                    TRIG_RADAR,
                    ECHO_RADAR
                );


            Estado.sensores.radarOK =
                true;



            pasoActual =
                PasoRadar::IR_IZQUIERDA;


        break;


    }



}







void Radar::mover(
    uint8_t posicion
)
{

    servoRadar.write(
        posicion
    );

}



#endif