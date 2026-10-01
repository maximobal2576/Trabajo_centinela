/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Sensores.h
 **************************************************************************/
#ifndef SENSORES_H
#define SENSORES_H

class Sensores {
public:
    static void iniciar();
    static void actualizar();
    static int  leerDistancia(int trig, int echo);
};

#endif // SENSORES_H
