/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Bateria.h
 **************************************************************************/
#ifndef BATERIA_H
#define BATERIA_H

class Bateria {
public:
    static void iniciar();
    static void actualizar();

private:
    static int calcularPorcentaje(int lectura);
};

#endif // BATERIA_H
