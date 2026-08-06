#ifndef DECISION_CPP
#define DECISION_CPP


#include "Decision.h"

#include "Configuracion.h"

#include "Estado.h"



/*
======================================================

                CENTINELA V2

              DECISION V2

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



    // Si está seguro no seguimos
    if(
        Estado.sistema.modo ==
        ModoSistema::SEGURO
    )
    {

        return;

    }



    analizarRadar();


    analizarFrontal();


    analizarSuelo();


    analizarCaida();



}






void Decision::gestionarModo()
{


    // Sensor crítico desconectado

    if(
        !Estado.sensores.sensorFrontalOK
        ||
        !Estado.sensores.sensorSueloOK
    )
    {


        Estado.sistema.errorSensor =
            true;



        Estado.sistema.modo =
            ModoSistema::SEGURO;


    }



}







void Decision::analizarRadar()
{


    // Zona izquierda extrema 30 grados

    if(
        Estado.sensores.radar30
        <
        DISTANCIA_CRITICA
    )
    {

        Estado.salida.motorIzquierdo =
            VIBRACION_FUERTE;

    }



    // Zona izquierda cercana 60 grados

    if(
        Estado.sensores.radar60
        <
        DISTANCIA_CRITICA
    )
    {

        Estado.salida.motorIzquierdo =
            VIBRACION_MEDIA;

    }




    // Frente 90 grados

    if(
        Estado.sensores.radar90
        <
        DISTANCIA_CRITICA
    )
    {

        Estado.salida.motorCentro =
            VIBRACION_FUERTE;

    }





    // Derecha cercana 120 grados

    if(
        Estado.sensores.radar120
        <
        DISTANCIA_CRITICA
    )
    {

        Estado.salida.motorDerecho =
            VIBRACION_MEDIA;

    }





    // Derecha extrema 150 grados

    if(
        Estado.sensores.radar150
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
        !Estado.sensores.sensorFrontalOK
    )
    {

        return;

    }




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
        !Estado.sensores.sensorSueloOK
    )
    {

        return;

    }




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