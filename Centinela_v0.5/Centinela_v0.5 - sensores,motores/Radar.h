#ifndef RADAR_H
#define RADAR_H

#include <Arduino.h>
#include <Servo.h>


namespace Radar
{

void iniciar();

void actualizar();


int obtenerDistancia();

int obtenerAngulo();

int obtenerZona();
// Funciones internas

int medirDistancia();

int filtrar(int nueva);




}


#endif