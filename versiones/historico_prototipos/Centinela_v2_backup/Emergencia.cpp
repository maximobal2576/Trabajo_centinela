/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Emergencia.cpp
 * Descripcion:
 *   Deteccion con antirrebote (debouncing) del pulsador de emergencia.
 *   Al ser presionado, eleva el estado a EVENTO_EMERGENCIA con prioridad
 *   critica y notifica de inmediato por Bluetooth a la app para lanzar
 *   la llamada automatica al 911 / contacto de confianza con coordenadas GPS.
 **************************************************************************/
#include "Emergencia.h"
#include "Estado.h"
#include "Bluetooth.h"

unsigned long Emergencia::_ultimoCambio        = 0;
int           Emergencia::_ultimoEstadoFisico  = HIGH;
bool          Emergencia::_estadoConfirmado    = false;

void Emergencia::iniciar()
{
    pinMode(PIN_BOTON_SOS, INPUT_PULLUP);
    _ultimoEstadoFisico = digitalRead(PIN_BOTON_SOS);
    _estadoConfirmado   = (_ultimoEstadoFisico == NIVEL_SOS_PRESIONADO);
    Serial.println("Pulsador de Emergencia SOS iniciado en GPIO 15 (INPUT_PULLUP)");
}

bool Emergencia::botonPresionado()
{
    return _estadoConfirmado;
}

void Emergencia::actualizar()
{
    int lecturaFisica = digitalRead(PIN_BOTON_SOS);

    if (lecturaFisica != _ultimoEstadoFisico)
    {
        _ultimoCambio = millis();
        _ultimoEstadoFisico = lecturaFisica;
    }

    if ((millis() - _ultimoCambio) > SOS_DEBOUNCE_MS)
    {
        bool presionadoAhora = (lecturaFisica == NIVEL_SOS_PRESIONADO);

        if (presionadoAhora != _estadoConfirmado)
        {
            _estadoConfirmado = presionadoAhora;
            Estado::sensores.botonSosPresionado = _estadoConfirmado;
            Estado::sistema.botonSosPresionado  = _estadoConfirmado;

            if (_estadoConfirmado)
            {
                // Pulsador activado: activar estado de alerta critica
                Estado::sistema.eventoActual    = EVENTO_EMERGENCIA;
                Estado::sistema.estado          = SISTEMA_EMERGENCIA;
                Estado::sistema.prioridadActual = PRIORIDAD_CRITICA;

                // Notificacion inmediata por Bluetooth al celular
                if (Estado::sistema.bluetoothConectado)
                {
                    Bluetooth::enviarComando("EVENTO:EMERGENCIA");
                    Bluetooth::enviarComando("SOS:1");
                }

                Serial.println("[ALERTA] Boton de emergencia SOS activado por el usuario!");
            }
        }
    }
}
