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



    // Obstáculo frontal prioridad alta

    if(Estado::sensores.frontal <= DISTANCIA_ALERTA_MAX)
    {


        distancia = Estado::sensores.frontal;



        Estado::salida.motorFrontal =
        calcularIntensidad(distancia);



        Estado::sistema.eventoActual =
        EVENTO_OBSTACULO_FRONTAL;



        Estado::sistema.prioridadActual =
        PRIORIDAD_NORMAL;



        return;


    }







    // Radar real

    distancia = Radar::obtenerDistancia();



    if(distancia <= DISTANCIA_ALERTA_MAX)
    {


        int zona = Radar::obtenerZona();



        int intensidad =
        calcularIntensidad(distancia);




        switch(zona)
        {


            case 0:

                Estado::salida.motorIzquierdo =
                intensidad;

                break;




            case 1:

                Estado::salida.motorCentro =
                intensidad;

                break;




            case 2:

                Estado::salida.motorDerecho =
                intensidad;

                break;


        }




        Estado::sistema.eventoActual =
        EVENTO_OBSTACULO_RADAR;



        Estado::sistema.prioridadActual =
        PRIORIDAD_NORMAL;


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