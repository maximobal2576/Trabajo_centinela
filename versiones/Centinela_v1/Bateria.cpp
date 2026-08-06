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

    static int lecturaSuavizada = 0;

    int lectura = analogRead(PIN_BATERIA);

    if(lecturaSuavizada == 0)
    {
        lecturaSuavizada = lectura;
    }
    else
    {
        lecturaSuavizada =
            (lecturaSuavizada * 7 + lectura) / 8;
    }

    Estado::sistema.bateria =
    calcularPorcentaje(lecturaSuavizada);

    if(Estado::sistema.bateria < BATERIA_UMBRAL_BAJA)
    {
        Estado::sistema.eventoActual =
        EVENTO_BATERIA_BAJA;
        Estado::sistema.prioridadActual = PRIORIDAD_ALTA;
        Estado::sistema.direccion = DIRECCION_NINGUNA;
    }
    else if(Estado::sistema.bateria >= BATERIA_UMBRAL_RECUPERACION)
    {
        if(Estado::sistema.eventoActual == EVENTO_BATERIA_BAJA)
        {
            Estado::sistema.eventoActual =
            EVENTO_NORMAL;
            Estado::sistema.prioridadActual = PRIORIDAD_BAJA;
        }
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