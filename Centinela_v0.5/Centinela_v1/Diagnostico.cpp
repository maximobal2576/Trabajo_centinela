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

    static unsigned long ultimoDiagnostico = 0;

    if(millis() - ultimoDiagnostico < TIEMPO_DIAGNOSTICO)
    {
        return;
    }

    ultimoDiagnostico = millis();
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


    Serial.print("Ángulo Radar: ");

    Serial.println(
        Estado::sensores.anguloRadar
    );



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

    Serial.print("Dirección: ");

    Serial.println(
        Estado::sistema.direccion
    );

    Serial.print("Batería: ");

    Serial.print(
        Estado::sistema.bateria
    );

    Serial.println(" %");

    Serial.print("Bluetooth: ");

    Serial.println(
        Estado::sistema.bluetoothConectado ? 1 : 0
    );



    Serial.println("====================");

}