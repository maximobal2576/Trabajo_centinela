/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Decision.cpp
 * Descripción:
 * Lógica principal del sistema.
 **************************************************************************/


#include "Decision.h"

#include "Estado.h"



void Decision::iniciar()
{

    Estado::sistema.eventoActual = EVENTO_NORMAL;

    Estado::sistema.prioridadActual = PRIORIDAD_BAJA;

}



void Decision::actualizar()
{


    // Limpiar salida anterior

    Estado::salida.motorIzquierdo = 0;

    Estado::salida.motorCentro = 0;

    Estado::salida.motorDerecho = 0;

    Estado::salida.motorFrontal = 0;


    // Limpiar evento anterior
    if(!Estado::sensores.caidaDetectada)
{
    Estado::sistema.eventoActual = EVENTO_NORMAL;
    Estado::sistema.prioridadActual = PRIORIDAD_BAJA;
}

    // Orden de prioridades

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


    static unsigned long tiempoCaida = 0;


    if(Estado::sensores.caidaDetectada)
    {

        if(tiempoCaida == 0)
        {
            tiempoCaida = millis();
        }


        // Mantener alerta máximo 5 segundos

        if(millis() - tiempoCaida < 5000)
        {

            Estado::sistema.eventoActual = EVENTO_CAIDA;

            Estado::sistema.prioridadActual = PRIORIDAD_CRITICA;


            Estado::salida.reproducirAudio = true;

            Estado::salida.audioID = 1;

        }

    }
    else
    {

        tiempoCaida = 0;

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


    int intensidad;



    if(Estado::sensores.frontal <= DISTANCIA_ALERTA_MAX)
    {

        intensidad =
        calcularIntensidad(Estado::sensores.frontal);



        Estado::salida.motorFrontal = intensidad;



        Estado::sistema.eventoActual =
        EVENTO_OBSTACULO_FRONTAL;

    }



    if(Estado::sensores.radarIzquierda <= DISTANCIA_ALERTA_MAX)
    {

        Estado::salida.motorIzquierdo =
        calcularIntensidad(
        Estado::sensores.radarIzquierda);



        Estado::sistema.eventoActual =
        EVENTO_OBSTACULO_RADAR;

    }



    if(Estado::sensores.radarCentro <= DISTANCIA_ALERTA_MAX)
    {

        Estado::salida.motorCentro =
        calcularIntensidad(
        Estado::sensores.radarCentro);



        Estado::sistema.eventoActual =
        EVENTO_OBSTACULO_RADAR;

    }



    if(Estado::sensores.radarDerecha <= DISTANCIA_ALERTA_MAX)
    {

        Estado::salida.motorDerecho =
        calcularIntensidad(
        Estado::sensores.radarDerecha);



        Estado::sistema.eventoActual =
        EVENTO_OBSTACULO_RADAR;

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



    // Interpolación entre 80 y 20 cm

    int pwm =
    map(distancia, 80, 20, 80, 255);



    return constrain(pwm, 80, 255);

}