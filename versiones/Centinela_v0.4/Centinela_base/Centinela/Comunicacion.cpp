#ifndef COMUNICACION_CPP
#define COMUNICACION_CPP


#include "Comunicacion.h"

#include "Configuracion.h"
#include "Estado.h"



/*
======================================================

                CENTINELA V2

              COMUNICACION

  Archivo:
  Comunicacion.cpp


======================================================
*/



void Comunicacion::iniciar()
{

    Serial.begin(9600);


    Serial.println();

    Serial.println(
        "CENTINELA V2 INICIADO"
    );


}





void Comunicacion::actualizar()
{

    enviarEstado();


    recibirComando();


}





void Comunicacion::enviarEstado()
{

    if(!DEBUG_GENERAL)
    {
        return;
    }



    Serial.print("Modo: ");



    switch(
        Estado.sistema.modo
    )
    {


        case ModoSistema::NORMAL:

            Serial.println("NORMAL");

        break;



        case ModoSistema::PRUEBA:

            Serial.println("PRUEBA");

        break;



        case ModoSistema::DIAGNOSTICO:

            Serial.println("DIAGNOSTICO");

        break;



        case ModoSistema::CALIBRACION:

            Serial.println("CALIBRACION");

        break;



        case ModoSistema::SEGURO:

            Serial.println("SEGURO");

        break;


    }



    Serial.print(
        "Frontal: "
    );


    Serial.print(
        Estado.sensores.distanciaFrontal
    );


    Serial.println(
        " cm"
    );



}





void Comunicacion::recibirComando()
{


    if(
        Serial.available()
    )
    {

        char comando =
            Serial.read();



        /*
        
        Reservado para futuro:

        N -> modo normal
        D -> diagnostico
        C -> calibracion
        S -> seguro

        */


    }


}




#endif