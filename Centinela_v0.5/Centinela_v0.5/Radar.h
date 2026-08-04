#ifndef RADAR_H
#define RADAR_H

#include <Arduino.h>


namespace Radar
{

void iniciar();

void actualizar();


int obtenerDistancia();

int obtenerAngulo();

int obtenerZona();

int obtenerDistanciaZona(int zona);


}


#endif