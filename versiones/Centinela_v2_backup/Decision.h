/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Decision.h
 **************************************************************************/
#ifndef DECISION_H
#define DECISION_H

class Decision {
public:
    static void iniciar();
    static void actualizar();

private:
    static void analizarCaida();
    static void analizarSuelo();
    static void analizarObstaculos();
    static int  calcularIntensidad(int distancia);
};

#endif // DECISION_H
