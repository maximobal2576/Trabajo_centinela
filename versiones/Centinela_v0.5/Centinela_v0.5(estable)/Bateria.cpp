/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Bateria.cpp
 * Descripción:
 * Medición de batería.
 **************************************************************************/


#include "Bateria.h"

#include "Estado.h"



void Bateria::iniciar()
{

    pinMode(PIN_BATERIA, INPUT);

}



void Bateria::actualizar()
{


    int lectura =
    analogRead(PIN_BATERIA);



    Estado::sistema.bateria =
    calcularPorcentaje(lectura);



    if(
        Estado::sistema.bateria < 15
    )
    {

        Estado::sistema.eventoActual =
        EVENTO_BATERIA_BAJA;

    }



}



int Bateria::calcularPorcentaje(int lectura)
{


    /*
       Conversión inicial.

       Se calibrará con el voltaje real
       de la batería y divisor usado.
    */


    int porcentaje =
    map(lectura, 600, 900, 0, 100);



    return constrain(
        porcentaje,
        0,
        100
    );


}