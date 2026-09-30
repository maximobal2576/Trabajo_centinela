/**
 * @file    Emergencia.cpp
 * @brief   Implementación del botón de pánico SOS — Centinela v2.0
 */

#include "Emergencia.h"
#include "Estado.h"
#include "Bluetooth.h"

uint32_t Emergencia::_ultimoCambio = 0;
int      Emergencia::_ultimoEstadoFisico = HIGH;
bool     Emergencia::_estadoConfirmado = false;

void Emergencia::iniciar() {
    pinMode(PIN_BOTON_SOS, INPUT_PULLUP);
    _ultimoEstadoFisico = digitalRead(PIN_BOTON_SOS);
    _estadoConfirmado = (_ultimoEstadoFisico == NIVEL_SOS_PRESIONADO);
    Serial.println(F("  [Emergencia] Botón SOS configurado en GPIO 15"));
}

bool Emergencia::botonPresionado() {
    return _estadoConfirmado;
}

void Emergencia::actualizar() {
    int lecturaFisica = digitalRead(PIN_BOTON_SOS);

    if (lecturaFisica != _ultimoEstadoFisico) {
        _ultimoCambio = millis();
        _ultimoEstadoFisico = lecturaFisica;
    }

    if ((millis() - _ultimoCambio) > SOS_DEBOUNCE_MS) {
        bool presionadoAhora = (lecturaFisica == NIVEL_SOS_PRESIONADO);

        if (presionadoAhora != _estadoConfirmado) {
            _estadoConfirmado = presionadoAhora;

            if (_estadoConfirmado) {
                // Activar emergencia en el estado global
                Estado::setEmergenciaSOS(true);

                // Notificar por Bluetooth si está disponible
                Bluetooth::enviarComando(F("EVENTO:EMERGENCIA"));
                Bluetooth::enviarComando(F("SOS:1"));
                Serial.println(F("  [Emergencia] ¡BOTÓN SOS ACTIVADO!"));
            }
        }
    }
}
