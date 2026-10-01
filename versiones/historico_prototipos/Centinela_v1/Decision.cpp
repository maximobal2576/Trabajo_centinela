/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Decision.cpp
 * Descripción:
 * Lógica principal del sistema y activación de motores.
 **************************************************************************/

#include "Decision.h"
#include "Estado.h"

void Decision::iniciar()
{
    Estado::sistema.eventoActual = EVENTO_NORMAL;
    Estado::sistema.prioridadActual = PRIORIDAD_BAJA;
}

void Decision::actualizar()
{
    // Limpiar salida anterior
    Estado::salida.motorIzquierdo = 0;
    Estado::salida.motorCentro = 0;
    Estado::salida.motorDerecho = 0;
    Estado::salida.motorFrontal = 0;

    // Resetear a NORMAL por defecto en cada ciclo si no hay condición crítica sostenida
    if(!Estado::sensores.caidaDetectada)
    {
        if(Estado::sistema.eventoActual != EVENTO_BATERIA_BAJA &&
           Estado::sistema.eventoActual != EVENTO_ERROR)
        {
            Estado::sistema.eventoActual = EVENTO_NORMAL;
            Estado::sistema.prioridadActual = PRIORIDAD_BAJA;
            Estado::sistema.direccion = DIRECCION_NINGUNA;
        }
    }

    // 1. Analizar caída (máxima prioridad)
    analizarCaida();

    // 2. Analizar suelo / pozo
    if(Estado::sistema.eventoActual != EVENTO_CAIDA)
    {
        analizarSuelo();
    }

    // 3. Analizar obstáculos frontal y radar (activa motores hápticos)
    if(Estado::sistema.eventoActual != EVENTO_CAIDA)
    {
        analizarObstaculos();
    }

    // 4. Actualizar estado general del sistema
    if(Estado::sistema.estado != SISTEMA_DIAGNOSTICO &&
       Estado::sistema.estado != SISTEMA_ERROR)
    {
        if(Estado::sistema.eventoActual == EVENTO_CAIDA)
        {
            Estado::sistema.estado = SISTEMA_CAIDA;
        }
        else if(Estado::sistema.eventoActual == EVENTO_POZO)
        {
            Estado::sistema.estado = SISTEMA_POZO;
        }
        else if(Estado::sistema.eventoActual == EVENTO_OBSTACULO_FRONTAL ||
                Estado::sistema.eventoActual == EVENTO_OBSTACULO_RADAR)
        {
            Estado::sistema.estado = SISTEMA_ALERTA;
        }
        else if(Estado::sistema.eventoActual == EVENTO_BATERIA_BAJA)
        {
            Estado::sistema.estado = SISTEMA_ALERTA;
        }
        else
        {
            Estado::sistema.estado = SISTEMA_NORMAL;
        }
    }
}

// =====================================================
// CAIDA
// =====================================================

void Decision::analizarCaida()
{
    static unsigned long tiempoCaida = 0;

    if(Estado::sensores.caidaDetectada)
    {
        if(tiempoCaida == 0)
        {
            tiempoCaida = millis();
        }

        // Mantener alerta máximo 5 segundos
        if(millis() - tiempoCaida < 5000)
        {
            Estado::sistema.eventoActual = EVENTO_CAIDA;
            Estado::sistema.prioridadActual = PRIORIDAD_CRITICA;
            Estado::sistema.direccion = DIRECCION_SUELO;

            Estado::salida.reproducirAudio = true;
            Estado::salida.audioID = 1;
        }
    }
    else
    {
        tiempoCaida = 0;
    }
}

// =====================================================
// SUELO / POZO
// =====================================================

void Decision::analizarSuelo()
{
    // Sensor de suelo apunta hacia abajo (distancia normal ~10-25 cm).
    // Si hay un pozo o desnivel, la distancia al piso aumenta (> 35 cm)
    if(Estado::sensores.suelo > 35 && Estado::sensores.suelo < 350)
    {
        Estado::sistema.eventoActual = EVENTO_POZO;
        Estado::sistema.prioridadActual = PRIORIDAD_ALTA;
        Estado::sistema.direccion = DIRECCION_SUELO;

        Estado::salida.motorIzquierdo = PWM_POZO;
        Estado::salida.motorCentro = PWM_POZO;
        Estado::salida.motorDerecho = PWM_POZO;
    }
}

// =====================================================
// OBSTACULOS (FRONTAL + RADAR)
// =====================================================

void Decision::analizarObstaculos()
{
    bool pozoActivo = (Estado::sistema.eventoActual == EVENTO_POZO);

    // Sensor Frontal
    if(Estado::sensores.frontal <= DISTANCIA_ALERTA_MAX)
    {
        int intensidad = calcularIntensidad(Estado::sensores.frontal);
        Estado::salida.motorFrontal = intensidad;

        if(!pozoActivo)
        {
            Estado::sistema.eventoActual = EVENTO_OBSTACULO_FRONTAL;
            Estado::sistema.direccion = DIRECCION_FRONTAL;
        }
    }

    // Radar Inferior (Izquierda, Centro, Derecha)
    bool radarIzquierda = (Estado::sensores.radarIzquierda <= DISTANCIA_ALERTA_MAX);
    bool radarCentro    = (Estado::sensores.radarCentro <= DISTANCIA_ALERTA_MAX);
    bool radarDerecha   = (Estado::sensores.radarDerecha <= DISTANCIA_ALERTA_MAX);

    if (radarIzquierda && radarCentro)
    {
        Estado::salida.motorIzquierdo = calcularIntensidad(Estado::sensores.radarIzquierda);
        Estado::salida.motorCentro    = calcularIntensidad(Estado::sensores.radarCentro);
        if(!pozoActivo)
        {
            Estado::sistema.eventoActual = EVENTO_OBSTACULO_RADAR;
            Estado::sistema.direccion = DIRECCION_DIAGONAL_IZQUIERDA;
        }
    }
    else if (radarCentro && radarDerecha)
    {
        Estado::salida.motorCentro  = calcularIntensidad(Estado::sensores.radarCentro);
        Estado::salida.motorDerecho = calcularIntensidad(Estado::sensores.radarDerecha);
        if(!pozoActivo)
        {
            Estado::sistema.eventoActual = EVENTO_OBSTACULO_RADAR;
            Estado::sistema.direccion = DIRECCION_DIAGONAL_DERECHA;
        }
    }
    else
    {
        if (radarIzquierda)
        {
            Estado::salida.motorIzquierdo = calcularIntensidad(Estado::sensores.radarIzquierda);
            if(!pozoActivo)
            {
                Estado::sistema.eventoActual = EVENTO_OBSTACULO_RADAR;
                Estado::sistema.direccion = DIRECCION_IZQUIERDA;
            }
        }

        if (radarCentro)
        {
            Estado::salida.motorCentro = calcularIntensidad(Estado::sensores.radarCentro);
            if(!pozoActivo)
            {
                Estado::sistema.eventoActual = EVENTO_OBSTACULO_RADAR;
                Estado::sistema.direccion = DIRECCION_CENTRO;
            }
        }

        if (radarDerecha)
        {
            Estado::salida.motorDerecho = calcularIntensidad(Estado::sensores.radarDerecha);
            if(!pozoActivo)
            {
                Estado::sistema.eventoActual = EVENTO_OBSTACULO_RADAR;
                Estado::sistema.direccion = DIRECCION_DERECHA;
            }
        }
    }

    if(pozoActivo)
    {
        if(Estado::salida.motorIzquierdo < PWM_POZO) Estado::salida.motorIzquierdo = PWM_POZO;
        if(Estado::salida.motorCentro < PWM_POZO)    Estado::salida.motorCentro = PWM_POZO;
        if(Estado::salida.motorDerecho < PWM_POZO)   Estado::salida.motorDerecho = PWM_POZO;
        Estado::sistema.eventoActual = EVENTO_POZO;
        Estado::sistema.prioridadActual = PRIORIDAD_ALTA;
        Estado::sistema.direccion = DIRECCION_SUELO;
    }
}

// =====================================================
// DISTANCIA → PWM
// =====================================================

int Decision::calcularIntensidad(int distancia)
{
    if(distancia >= DISTANCIA_ALERTA_MAX)
    {
        return 0;
    }

    if(distancia <= DISTANCIA_PELIGRO)
    {
        return PWM_MAXIMO;
    }

    // Mapeo proporcional entre 80cm y 20cm (80 a 255 PWM)
    int pwm = map(distancia, DISTANCIA_ALERTA_MAX, DISTANCIA_PELIGRO, PWM_SUAVE, PWM_MAXIMO);
    return constrain(pwm, PWM_SUAVE, PWM_MAXIMO);
}
