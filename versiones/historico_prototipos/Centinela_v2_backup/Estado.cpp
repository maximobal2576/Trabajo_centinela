/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Estado.cpp
 * Descripcion: Inicializacion del estado global con soporte para todos
 *              los componentes del hardware v2.
 **************************************************************************/
#include "Estado.h"
#include "Configuracion.h"

DatosSensores Estado::sensores;
DatosSalida   Estado::salida;
DatosSistema  Estado::sistema;
DatosApp      Estado::app;

void Estado::iniciar()
{
    // Sensores
    sensores.frontal             = 400;
    sensores.suelo               = 400;
    sensores.radarIzquierda      = 400;
    sensores.radarCentro         = 400;
    sensores.radarDerecha        = 400;
    sensores.anguloRadar         = RADAR_IZQUIERDA;
    sensores.inclinacionX        = 0;
    sensores.inclinacionY        = 0;
    sensores.inclinacionZ        = 0;
    sensores.movimientoDetectado = false;
    sensores.caidaDetectada      = false;
    sensores.botonSosPresionado  = false;

    // Salidas (3 motores vibradores hápticos)
    salida.motorIzquierdo        = 0;
    salida.motorCentro           = 0;
    salida.motorDerecho          = 0;
    salida.reproducirAudio       = false;
    salida.volumenI2S            = 200;

    // Sistema
    sistema.estado                = SISTEMA_INICIO;
    sistema.eventoActual          = EVENTO_NORMAL;
    sistema.prioridadActual       = PRIORIDAD_BAJA;
    sistema.direccion             = DIRECCION_NINGUNA;
    sistema.bateria               = 100;
    sistema.bluetoothConectado    = false;
    sistema.audioStreamActivo     = false;
    sistema.ultimoChunkAudio      = 0;
    sistema.auricularesConectados = false;
    sistema.botonSosPresionado    = false;
    sistema.gpsLatitud            = 0.0f;
    sistema.gpsLongitud           = 0.0f;
    sistema.gpsFijado             = false;

    // App
    app.navegacionActiva       = false;
    app.gpsDisponible          = false;
    app.microfonoActivo        = false;
    app.recorridoOfflineActivo = false;
}
