/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Decision.cpp
 * Descripcion:
 *   Logica principal de toma de decisiones para los 3 motores vibradores
 *   (Izquierdo, Centro, Derecho) segun sensores ultrasonicos, caida MPU6050
 *   y pulsador de emergencia SOS.
 **************************************************************************/
#include "Decision.h"
#include "Estado.h"
#include "Configuracion.h"

void Decision::iniciar()
{
    Estado::sistema.eventoActual    = EVENTO_NORMAL;
    Estado::sistema.prioridadActual = PRIORIDAD_BAJA;
}

void Decision::actualizar()
{
    // Limpiar salida de los 3 motores vibradores
    Estado::salida.motorIzquierdo = 0;
    Estado::salida.motorCentro    = 0;
    Estado::salida.motorDerecho   = 0;

    // 0. Pulsador de emergencia SOS (Maxima prioridad del usuario)
    if (Estado::sistema.botonSosPresionado || Estado::sensores.botonSosPresionado)
    {
        Estado::sistema.eventoActual    = EVENTO_EMERGENCIA;
        Estado::sistema.estado          = SISTEMA_EMERGENCIA;
        Estado::sistema.prioridadActual = PRIORIDAD_CRITICA;
        return;
    }

    // Resetear a NORMAL si no hay condicion critica sostenida
    if (!Estado::sensores.caidaDetectada)
    {
        if (Estado::sistema.eventoActual != EVENTO_BATERIA_BAJA &&
            Estado::sistema.eventoActual != EVENTO_ERROR)
        {
            Estado::sistema.eventoActual    = EVENTO_NORMAL;
            Estado::sistema.prioridadActual = PRIORIDAD_BAJA;
            Estado::sistema.direccion       = DIRECCION_NINGUNA;
        }
    }

    // 1. Caida (detectada por MPU-6050)
    analizarCaida();

    // 2. Suelo / pozo (desnivel)
    if (Estado::sistema.eventoActual != EVENTO_CAIDA)
    {
        analizarSuelo();
    }

    // 3. Obstaculos frontal y radar
    if (Estado::sistema.eventoActual != EVENTO_CAIDA)
    {
        analizarObstaculos();
    }

    // 4. Actualizar estado general del sistema
    if (Estado::sistema.estado != SISTEMA_DIAGNOSTICO &&
        Estado::sistema.estado != SISTEMA_ERROR)
    {
        if (Estado::sistema.eventoActual == EVENTO_CAIDA)
            Estado::sistema.estado = SISTEMA_CAIDA;
        else if (Estado::sistema.eventoActual == EVENTO_POZO)
            Estado::sistema.estado = SISTEMA_POZO;
        else if (Estado::sistema.eventoActual == EVENTO_OBSTACULO_FRONTAL ||
                 Estado::sistema.eventoActual == EVENTO_OBSTACULO_RADAR)
            Estado::sistema.estado = SISTEMA_ALERTA;
        else if (Estado::sistema.eventoActual == EVENTO_BATERIA_BAJA)
            Estado::sistema.estado = SISTEMA_ALERTA;
        else
            Estado::sistema.estado = SISTEMA_NORMAL;
    }
}

// =====================================================
// CAIDA (MPU-6050)
// =====================================================

void Decision::analizarCaida()
{
    if (Estado::sensores.caidaDetectada)
    {
        Estado::sistema.eventoActual    = EVENTO_CAIDA;
        Estado::sistema.prioridadActual = PRIORIDAD_CRITICA;
        Estado::sistema.direccion       = DIRECCION_NINGUNA;

        // Vibracion de alerta en los 3 motores
        Estado::salida.motorIzquierdo = PWM_MAXIMO;
        Estado::salida.motorCentro    = PWM_MAXIMO;
        Estado::salida.motorDerecho   = PWM_MAXIMO;
    }
}

// =====================================================
// SUELO / POZO
// =====================================================

void Decision::analizarSuelo()
{
    int d = Estado::sensores.suelo;

    // Distancia mayor al umbral indica hueco o caida de nivel
    if (d > DISTANCIA_ALERTA_MAX)
    {
        Estado::sistema.eventoActual    = EVENTO_POZO;
        Estado::sistema.prioridadActual = PRIORIDAD_ALTA;
        Estado::sistema.direccion       = DIRECCION_SUELO;

        // Vibracion central fuerte para advertir pozo
        Estado::salida.motorCentro = PWM_POZO;
    }
}

// =====================================================
// OBSTACULOS (FRONTAL + RADAR SERVO)
// =====================================================

void Decision::analizarObstaculos()
{
    bool pozoActivo = (Estado::sistema.eventoActual == EVENTO_POZO);

    // Sensor Frontal
    if (Estado::sensores.frontal <= DISTANCIA_ALERTA_MAX)
    {
        int intensidad = calcularIntensidad(Estado::sensores.frontal);
        Estado::salida.motorCentro = max(Estado::salida.motorCentro, intensidad);

        if (!pozoActivo)
        {
            Estado::sistema.eventoActual = EVENTO_OBSTACULO_FRONTAL;
            Estado::sistema.direccion    = DIRECCION_FRONTAL;
        }
    }

    // Radar de barrido angular
    bool rIzq = (Estado::sensores.radarIzquierda <= DISTANCIA_ALERTA_MAX);
    bool rCtr = (Estado::sensores.radarCentro    <= DISTANCIA_ALERTA_MAX);
    bool rDer = (Estado::sensores.radarDerecha   <= DISTANCIA_ALERTA_MAX);

    if (rIzq || rCtr || rDer)
    {
        if (rIzq) Estado::salida.motorIzquierdo = calcularIntensidad(Estado::sensores.radarIzquierda);
        if (rCtr) Estado::salida.motorCentro    = max(Estado::salida.motorCentro, calcularIntensidad(Estado::sensores.radarCentro));
        if (rDer) Estado::salida.motorDerecho   = calcularIntensidad(Estado::sensores.radarDerecha);

        if (!pozoActivo && Estado::sistema.eventoActual != EVENTO_OBSTACULO_FRONTAL)
        {
            Estado::sistema.eventoActual = EVENTO_OBSTACULO_RADAR;

            if (rIzq && !rCtr && !rDer)      Estado::sistema.direccion = DIRECCION_IZQUIERDA;
            else if (rDer && !rCtr && !rIzq) Estado::sistema.direccion = DIRECCION_DERECHA;
            else if (rCtr && !rIzq && !rDer) Estado::sistema.direccion = DIRECCION_CENTRO;
            else if (rIzq && rCtr && !rDer)  Estado::sistema.direccion = DIRECCION_DIAGONAL_IZQUIERDA;
            else if (rDer && rCtr && !rIzq)  Estado::sistema.direccion = DIRECCION_DIAGONAL_DERECHA;
            else                             Estado::sistema.direccion = DIRECCION_FRONTAL;
        }
    }
}

// =====================================================
// CALCULAR INTENSIDAD PWM SEGUN PROXIMIDAD
// =====================================================

int Decision::calcularIntensidad(int distancia)
{
    if (distancia <= DISTANCIA_PELIGRO)    return PWM_MAXIMO;
    if (distancia >= DISTANCIA_ALERTA_MAX) return PWM_SUAVE;

    return map(distancia, DISTANCIA_ALERTA_MAX, DISTANCIA_PELIGRO, PWM_SUAVE, PWM_MAXIMO);
}
