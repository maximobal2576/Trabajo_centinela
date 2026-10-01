/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Diagnostico.h
 **************************************************************************/
#ifndef DIAGNOSTICO_H
#define DIAGNOSTICO_H

class Diagnostico {
public:
    static void iniciar();
    static void actualizar();

private:
    static void imprimirEstado();
};

#endif // DIAGNOSTICO_H
