#ifndef DECISION_CPP
#define DECISION_CPP


#include "Decision.h"

#include "Configuracion.h"
#include "Estado.h"



/*
======================================================

                CENTINELA V2

                  DECISION

  Archivo:
  Decision.cpp


======================================================
*/



void Decision::iniciar()
{

    Estado.sistema.modo =
        ModoSistema::NORMAL;

}




void Decision::actualizar()
{

    gestionarModo();


    analizarRadar();


    analizarFrontal();


    analizarSuelo();


    analizarCaida();


}





void Decision::gestionarModo()
{


    if(
        Estado.sistema.errorSensor ||
        Estado.sistema.errorRadar
    )
    {

        Estado.sistema.modo =
            ModoSistema::SEGURO;

    }


}




void Decision::analizarRadar()
{

    if(
        Estado.sensores.distanciaIzquierda
        <
        DISTANCIA_CRITICA
    )
    {

        Estado.salida.motorIzquierdo =
            VIBRACION_FUERTE;

    }



    if(
        Estado.sensores.distanciaCentro
        <
        DISTANCIA_CRITICA
    )
    {

        Estado.salida.motorCentro =
            VIBRACION_FUERTE;

    }



    if(
        Estado.sensores.distanciaDerecha
        <
        DISTANCIA_CRITICA
    )
    {

        Estado.salida.motorDerecho =
            VIBRACION_FUERTE;

    }


}





void Decision::analizarFrontal()
{


    if(
        Estado.sensores.distanciaFrontal
        <
        DISTANCIA_CRITICA
    )
    {

        Estado.salida.motorFrontal =
            VIBRACION_FUERTE;

    }


}





void Decision::analizarSuelo()
{


    if(
        Estado.sensores.distanciaSuelo
        <
        DISTANCIA_PRECAUCION
    )
    {

        Estado.salida.motorIzquierdo =
            VIBRACION_MEDIA;


        Estado.salida.motorCentro =
            VIBRACION_MEDIA;


        Estado.salida.motorDerecho =
            VIBRACION_MEDIA;


    }


}





void Decision::analizarCaida()
{


    if(
        Estado.sensores.caidaDetectada
    )
    {

        Estado.sistema.modo =
            ModoSistema::SEGURO;


    }



}



#endif