/**
 * @file    Sensores.cpp
 * @brief   Implementación del subsistema de sensores ultrasónicos HC-SR04.
 *
 * Controla los pines GPIO del ESP32 para generar los pulsos de disparo,
 * medir el tiempo de eco y calcular la distancia en centímetros.
 * Publica los resultados en Estado mediante la API centralizada.
 *
 * ─── PROTECCIÓN EN CALLE / VIDA REAL ─────────────────────────────────────────
 *  • Timeout de 15 ms en pulseIn (~2.55 m máx) para impedir que el CPU se
 *    congele si no hay eco o si hay lluvia/viento dispersando la onda.
 *  • Si no hay eco, se asume espacio libre (9999 cm) y se marca lectura inválida
 *    para que Decision no tome decisiones erróneas por ruido.
 * ─────────────────────────────────────────────────────────────────────────────
 */

#include "Sensores.h"
#include "Estado.h"
#include "Configuracion.h"

// ═══════════════════════════════════════════════════════════════════
// MÉTODOS PÚBLICOS
// ═══════════════════════════════════════════════════════════════════

void Sensores::iniciar() {
    // --- Sensor Frontal (torso / cabeza) ---
    pinMode(TRIG_FRONTAL, OUTPUT);
    pinMode(ECHO_FRONTAL, INPUT);
    digitalWrite(TRIG_FRONTAL, LOW);

    // --- Sensor de Suelo (pozos / escalones) ---
    pinMode(TRIG_SUELO, OUTPUT);
    pinMode(ECHO_SUELO, INPUT);
    digitalWrite(TRIG_SUELO, LOW);

    // --- Sensor del Radar (sobre servomotor SG90) ---
    pinMode(TRIG_RADAR, OUTPUT);
    pinMode(ECHO_RADAR, INPUT);
    digitalWrite(TRIG_RADAR, LOW);

    Serial.println(F("  [Sensores] HC-SR04 inicializados (Frontal / Suelo / Radar)"));
}

void Sensores::actualizar() {
    // 1. Lectura del sensor frontal
    int distFrontal = leerDistancia(TRIG_FRONTAL, ECHO_FRONTAL);
    if (distFrontal < 400 && distFrontal >= 2) {
        Estado::setDistanciaFrontal((uint16_t)distFrontal);
        Estado::setFrontalValido(true);
    } else {
        Estado::setDistanciaFrontal(9999);
        Estado::setFrontalValido(false);
    }

    // 2. Lectura del sensor de suelo
    int distSuelo = leerDistancia(TRIG_SUELO, ECHO_SUELO);
    if (distSuelo < 400 && distSuelo >= 2) {
        Estado::setDistanciaSuelo((uint16_t)distSuelo);
        Estado::setSueloValido(true);
    } else {
        Estado::setDistanciaSuelo(9999);
        Estado::setSueloValido(false);
    }
}

int Sensores::leerDistancia(int trig, int echo) {
    // 1. Asegurar señal limpia antes del disparo
    digitalWrite(trig, LOW);
    delayMicroseconds(2);

    // 2. Pulso de disparo de 10 microsegundos
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    // 3. Medición del eco con timeout seguro
    unsigned long duracion = pulseIn(echo, HIGH, 15000UL);

    if (duracion == 0) return 400; // Sin eco dentro de rango

    // 4. Conversión acústica: ida y vuelta a ~340 m/s
    int distancia = (int)(duracion * 0.034f / 2.0f);

    if (distancia < 2 || distancia > 400) {
        distancia = 400;
    }

    return distancia;
}
