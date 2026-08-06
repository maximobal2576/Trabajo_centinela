/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Tipos.h
 * Descripción:
 * Definiciones generales del sistema.
 *
 * Contiene:
 * - Eventos
 * - Estados
 * - Direcciones
 * - Estructuras de datos
 **************************************************************************/

#ifndef TIPOS_H
#define TIPOS_H


// =====================================================
// EVENTOS DEL SISTEMA
// =====================================================

enum Evento
{
    EVENTO_NORMAL,

    EVENTO_OBSTACULO_FRONTAL,

    EVENTO_OBSTACULO_RADAR,

    EVENTO_POZO,

    EVENTO_CAIDA,

    EVENTO_BATERIA_BAJA,

    EVENTO_ERROR
};



// =====================================================
// ESTADOS GENERALES
// =====================================================

enum EstadoSistema
{
    SISTEMA_INICIO,

    SISTEMA_NORMAL,

    SISTEMA_ALERTA,

    SISTEMA_POZO,

    SISTEMA_CAIDA,

    SISTEMA_DIAGNOSTICO,

    SISTEMA_ERROR
};



// =====================================================
// DIRECCION DEL PELIGRO
// =====================================================

enum Direccion
{
    DIRECCION_NINGUNA,

    DIRECCION_IZQUIERDA,

    DIRECCION_DIAGONAL_IZQUIERDA,

    DIRECCION_CENTRO,

    DIRECCION_DIAGONAL_DERECHA,

    DIRECCION_DERECHA,

    DIRECCION_FRONTAL,

    DIRECCION_SUELO
};



// =====================================================
// NIVEL DE PRIORIDAD
// =====================================================

enum Prioridad
{
    PRIORIDAD_BAJA = 0,

    PRIORIDAD_NORMAL = 1,

    PRIORIDAD_ALTA = 2,

    PRIORIDAD_CRITICA = 3
};



// =====================================================
// DATOS DE SENSORES
// =====================================================

struct DatosSensores
{  
    int anguloRadar;
    int frontal;

    int suelo;

    int radarIzquierda;

    int radarCentro;

    int radarDerecha;


    float inclinacionX;

    float inclinacionY;

    float inclinacionZ;


    bool movimientoDetectado;

    bool caidaDetectada;
};



// =====================================================
// DATOS DE SALIDA
// =====================================================

struct DatosSalida
{
    int motorIzquierdo;

    int motorCentro;

    int motorDerecho;

    int motorFrontal;


    bool reproducirAudio;

    int audioID;
};



// =====================================================
// DATOS DEL SISTEMA
// =====================================================

struct DatosSistema
{
    EstadoSistema estado;

    Evento eventoActual;

    Prioridad prioridadActual;

    Direccion direccion;

    int bateria;

    bool bluetoothConectado;

    bool audioOcupado;

    int audioPendienteID;
};



// =====================================================
// DATOS DE LA APP
// =====================================================

struct DatosApp
{
    bool navegacionActiva;

    bool gpsDisponible;

    bool microfonoActivo;

    bool recorridoOfflineActivo;
};



#endif