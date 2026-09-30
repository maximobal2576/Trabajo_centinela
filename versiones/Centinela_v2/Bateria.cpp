/**
 * @file    Bateria.cpp
 * @brief   Implementación del monitoreo de batería con filtro IIR y anti-rebote — Centinela v2.0
 */

#include "Bateria.h"
#include "Estado.h"

uint8_t Bateria::_porcentaje = 100;
int     Bateria::_lecturaFiltrada = 0;

void Bateria::iniciar() {
    pinMode(PIN_BATERIA, INPUT);
    analogReadResolution(12); // 0 a 4095

    // Lectura inicial para arrancar el filtro sin delay
    _lecturaFiltrada = analogRead(PIN_BATERIA);
    _porcentaje = _calcularPorcentaje(_lecturaFiltrada);
    Estado::setNivelBateria(_porcentaje);
    Estado::setBateriaAlarmaBaja(_porcentaje < BATERIA_UMBRAL_BAJA);

    Serial.print(F("  [Bateria] Nivel inicial: "));
    Serial.print(_porcentaje);
    Serial.println(F("%"));
}

uint8_t Bateria::getPorcentaje() {
    return _porcentaje;
}

uint8_t Bateria::_calcularPorcentaje(int lecturaAdc) {
    int pct = map(lecturaAdc, ADC_LECTURA_VACIA, ADC_LECTURA_LLENA, 0, 100);
    return (uint8_t)constrain(pct, 0, 100);
}

void Bateria::actualizar() {
    int lectura = analogRead(PIN_BATERIA);

    // Filtro IIR paso bajo (7/8 previo + 1/8 nuevo) para ignorar caídas por vibración de motores
    _lecturaFiltrada = (_lecturaFiltrada * 7 + lectura) / 8;
    _porcentaje = _calcularPorcentaje(_lecturaFiltrada);

    Estado::setNivelBateria(_porcentaje);

    // Histéresis para evitar oscilación de alarmas
    if (_porcentaje < BATERIA_UMBRAL_BAJA) {
        Estado::setBateriaAlarmaBaja(true);
    } else if (_porcentaje >= BATERIA_UMBRAL_RECUPERACION) {
        Estado::setBateriaAlarmaBaja(false);
    }
}
