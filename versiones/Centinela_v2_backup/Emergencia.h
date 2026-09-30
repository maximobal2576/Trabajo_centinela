/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Emergencia.h
 * Descripcion:
 *   Gestion del pulsador fisico de emergencia (Boton SOS).
 *   Maneja debouncing por software, notificacion inmediata por Bluetooth
 *   a la App Android y activacion de alerta haptica.
 **************************************************************************/
#ifndef EMERGENCIA_H
#define EMERGENCIA_H

#include "Configuracion.h"

class Emergencia {
public:
    static void iniciar();
    static void actualizar();
    static bool botonPresionado();

private:
    static unsigned long _ultimoCambio;
    static int           _ultimoEstadoFisico;
    static bool          _estadoConfirmado;
};

#endif // EMERGENCIA_H
