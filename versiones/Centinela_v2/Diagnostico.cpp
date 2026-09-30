/**
 * @file    Diagnostico.cpp
 * @brief   Implementación del módulo de diagnóstico por consola Serial — Centinela v2.0
 */

#include "Diagnostico.h"
#include "Estado.h"

void Diagnostico::iniciar() {
    Serial.println(F("========================================"));
    Serial.println(F("  Diagnóstico Centinela v2.0 Iniciado   "));
    Serial.println(F("========================================"));
}

void Diagnostico::actualizar() {
    Estado::imprimirDiagnostico();
}
