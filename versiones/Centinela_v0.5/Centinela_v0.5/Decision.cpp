/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Decision.cpp
 * Descripción:
 * Lógica principal del sistema.
 **************************************************************************/


#include "Decision.h"

#include "Estado.h"
#include "Radar.h"



void Decision::iniciar()
{

    Estado::sistema.eventoActual = EVENTO_NORMAL;

    Estado::sistema.prioridadActual = PRIORIDAD_BAJA;

}




void Decision::actualizar()
{

    // limpiar salidas anteriores

    Estado::salida.motorIzquierdo = 0;
    Estado::salida.motorCentro = 0;
    Estado::salida.motorDerecho = 0;
    Estado::salida.motorFrontal = 0;



    Estado::salida.reproducirAudio = false;



    if(!Estado::sensores.caidaDetectada)
    {
        Estado::sistema.eventoActual = EVENTO_NORMAL;
        Estado::sistema.prioridadActual = PRIORIDAD_BAJA;
    }



    analizarCaida();



    if(Estado::sistema.eventoActual != EVENTO_CAIDA)
    {
        analizarSuelo();
    }



    if(Estado::sistema.eventoActual == EVENTO_NORMAL)
    {
        analizarObstaculos();
    }


}





// =====================================================
// CAIDA
// =====================================================


void Decision::analizarCaida()
{


    if(Estado::sensores.caidaDetectada)
    {


        Estado::sistema.eventoActual = EVENTO_CAIDA;

        Estado::sistema.prioridadActual = PRIORIDAD_CRITICA;



        Estado::salida.reproducirAudio = true;

        Estado::salida.audioID = 1;


    }


}






// =====================================================
// SUELO / POZO
// =====================================================


void Decision::analizarSuelo()
{


    if(Estado::sensores.suelo < 20)
    {


        Estado::sistema.eventoActual = EVENTO_POZO;

        Estado::sistema.prioridadActual = PRIORIDAD_ALTA;



        Estado::salida.motorIzquierdo = PWM_POZO;

        Estado::salida.motorCentro = PWM_POZO;

        Estado::salida.motorDerecho = PWM_POZO;



    }


}







// =====================================================
// OBSTACULOS
// =====================================================


void Decision::analizarObstaculos()
{

    int distancia;

    int intensidad;



    //==================================================
    // Sensor frontal
    //==================================================

    if(Estado::sensores.frontal <= DISTANCIA_ALERTA_MAX)
    {

        intensidad =
        calcularIntensidad(
            Estado::sensores.frontal
        );


        Estado::salida.motorFrontal = intensidad;


        Estado::sistema.eventoActual =
        EVENTO_OBSTACULO_FRONTAL;


        Estado::sistema.prioridadActual =
        PRIORIDAD_ALTA;

    }



  //==================================================
// Radar 5 zonas con memoria
//==================================================


int zonas[5] =
{
    Radar::obtenerDistanciaZona(0),
    Radar::obtenerDistanciaZona(1),
    Radar::obtenerDistanciaZona(2),
    Radar::obtenerDistanciaZona(3),
    Radar::obtenerDistanciaZona(4)
};



for(int zona = 0; zona < 5; zona++)
{

    distancia = zonas[zona];


    if(distancia <= DISTANCIA_ALERTA_MAX)
    {

        intensidad =
        calcularIntensidad(distancia);



        switch(zona)
        {

            case 0:
            // izquierda
            Estado::salida.motorIzquierdo =
            max(
                Estado::salida.motorIzquierdo,
                intensidad
            );
            break;



            case 1:
            // diagonal izquierda

            Estado::salida.motorIzquierdo =
            max(
                Estado::salida.motorIzquierdo,
                intensidad
            );


            Estado::salida.motorCentro =
            max(
                Estado::salida.motorCentro,
                intensidad
            );

            break;



            case 2:
            // centro

            Estado::salida.motorCentro =
            max(
                Estado::salida.motorCentro,
                intensidad
            );

            break;



            case 3:
            // diagonal derecha

            Estado::salida.motorCentro =
            max(
                Estado::salida.motorCentro,
                intensidad
            );


            Estado::salida.motorDerecho =
            max(
                Estado::salida.motorDerecho,
                intensidad
            );

            break;



            case 4:
            // derecha

            Estado::salida.motorDerecho =
            max(
                Estado::salida.motorDerecho,
                intensidad
            );

            break;

        }



        Estado::sistema.eventoActual =
        EVENTO_OBSTACULO_RADAR;


        Estado::sistema.prioridadActual =
        PRIORIDAD_ALTA;

    }

}


}






// =====================================================
// DISTANCIA → PWM
// =====================================================


int Decision::calcularIntensidad(int distancia)
{


    if(distancia >= 80)
    {
        return 0;
    }




    if(distancia <= 20)
    {
        return PWM_MAXIMO;
    }




    int pwm =
    map(distancia,80,20,80,255);



    return constrain(pwm,80,255);


}