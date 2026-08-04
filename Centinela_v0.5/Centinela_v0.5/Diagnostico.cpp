/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Diagnostico.cpp
 * Descripción:
 * Monitor serial del estado interno.
 **************************************************************************/


#include "Diagnostico.h"

#include "Estado.h"

#include "Configuracion.h"

void Diagnostico::iniciar()
{

    Serial.println(
        "Diagnostico iniciado"
    );

}



void Diagnostico::actualizar()
{


    if(!MODO_DIAGNOSTICO)
    {

        return;

    }



    imprimirEstado();


}



void Diagnostico::imprimirEstado()
{


    Serial.println("====================");



    Serial.print("Frontal: ");

    Serial.print(
        Estado::sensores.frontal
    );

    Serial.println(" cm");



    Serial.print("Suelo: ");

    Serial.print(
        Estado::sensores.suelo
    );

    Serial.println(" cm");



    Serial.print("Radar Izq: ");

    Serial.print(
        Estado::sensores.radarIzquierda
    );

    Serial.println(" cm");



    Serial.print("Radar Centro: ");

    Serial.print(
        Estado::sensores.radarCentro
    );

    Serial.println(" cm");



    Serial.print("Radar Der: ");

    Serial.print(
        Estado::sensores.radarDerecha
    );

    Serial.println(" cm");



    Serial.print("Evento: ");

    Serial.println(
        Estado::sistema.eventoActual
    );



    Serial.println("====================");

}