/**
 * @file    Salida.cpp
 * @brief   Implementación del módulo de actuadores hápticos — Centinela v2.0
 *
 * Mapeo háptico direccional según la zona y nivel de peligro:
 *   - EMERGENCIA: pulso rítmico a máxima potencia en los 3 motores (alerta inequívoca).
 *   - CRÍTICO (Pozo): motor central activado con patrón distintivo.
 *   - CRÍTICO / PELIGRO / PRECAUCIÓN (Obstáculo): vibración graduada en el motor
 *     correspondiente al lado del obstáculo para que el usuario esquive naturalmente.
 */

#include "Salida.h"
#include "Estado.h"
#include "Decision.h"
#include <esp_arduino_version.h>

void Salida::iniciar() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(MOTOR_IZQUIERDO, LEDC_FREQ_HZ, LEDC_BITS);
    ledcAttach(MOTOR_CENTRO,    LEDC_FREQ_HZ, LEDC_BITS);
    ledcAttach(MOTOR_DERECHO,   LEDC_FREQ_HZ, LEDC_BITS);

    ledcWrite(MOTOR_IZQUIERDO, 0);
    ledcWrite(MOTOR_CENTRO,    0);
    ledcWrite(MOTOR_DERECHO,   0);
#else
    ledcSetup(LEDC_CH_IZQ, LEDC_FREQ_HZ, LEDC_BITS);
    ledcSetup(LEDC_CH_CTR, LEDC_FREQ_HZ, LEDC_BITS);
    ledcSetup(LEDC_CH_DER, LEDC_FREQ_HZ, LEDC_BITS);

    ledcAttachPin(MOTOR_IZQUIERDO, LEDC_CH_IZQ);
    ledcAttachPin(MOTOR_CENTRO,    LEDC_CH_CTR);
    ledcAttachPin(MOTOR_DERECHO,   LEDC_CH_DER);

    ledcWrite(LEDC_CH_IZQ, 0);
    ledcWrite(LEDC_CH_CTR, 0);
    ledcWrite(LEDC_CH_DER, 0);
#endif

    Serial.println(F("  [Salida] Motores hápticos inicializados"));
}

void Salida::detenerTodos() {
    escribirMotor(MOTOR_IZQUIERDO, LEDC_CH_IZQ, 0);
    escribirMotor(MOTOR_CENTRO,    LEDC_CH_CTR, 0);
    escribirMotor(MOTOR_DERECHO,   LEDC_CH_DER, 0);
}

void Salida::actualizar() {
    NivelAlerta nivel = Estado::getNivelAlerta();
    Zona        zona  = Estado::getZonaPeligro();

    // ── 1. PATRÓN DE EMERGENCIA (Caída o Botón SOS) ─────────────────────────
    if (nivel == NivelAlerta::EMERGENCIA || Estado::getEmergenciaSOS() || Estado::getCaidaConfirmada()) {
        static uint32_t tPulso = 0;
        static bool pulsoAlto = false;

        if (millis() - tPulso >= 180) {
            tPulso = millis();
            pulsoAlto = !pulsoAlto;
            uint8_t val = pulsoAlto ? PWM_MAXIMO : 0;
            escribirMotor(MOTOR_IZQUIERDO, LEDC_CH_IZQ, val);
            escribirMotor(MOTOR_CENTRO,    LEDC_CH_CTR, val);
            escribirMotor(MOTOR_DERECHO,   LEDC_CH_DER, val);
        }
        return;
    }

    // ── 2. SIN ALERTA ────────────────────────────────────────────────────────
    if (nivel == NivelAlerta::SIN_ALERTA) {
        detenerTodos();
        return;
    }

    // ── 3. SELECCIÓN DE INTENSIDAD BASE SEGÚN GRAVEDAD ───────────────────────
    uint8_t intensidad = 0;
    switch (nivel) {
        case NivelAlerta::CRITICO:    intensidad = PWM_MAXIMO; break;
        case NivelAlerta::PELIGRO:    intensidad = PWM_FUERTE; break;
        case NivelAlerta::PRECAUCION: intensidad = PWM_SUAVE;  break;
        default:                      intensidad = 0;          break;
    }

    // ── 4. DETECCIÓN DE POZO (AFECTA MOTOR CENTRAL CON PULSO O INTENSIDAD) ──
    if (Decision::hayPozo()) {
        escribirMotor(MOTOR_IZQUIERDO, LEDC_CH_IZQ, 0);
        escribirMotor(MOTOR_CENTRO,    LEDC_CH_CTR, PWM_POZO);
        escribirMotor(MOTOR_DERECHO,   LEDC_CH_DER, 0);
        return;
    }

    // ── 5. APLICACIÓN DIRECCIONAL A CADA MOTOR ──────────────────────────────
    uint8_t pwmIzq = 0;
    uint8_t pwmCtr = 0;
    uint8_t pwmDer = 0;

    switch (zona) {
        case Zona::IZQUIERDA:
            pwmIzq = intensidad;
            break;
        case Zona::CENTRO:
            pwmCtr = intensidad;
            break;
        case Zona::DERECHA:
            pwmDer = intensidad;
            break;
        case Zona::TODOS:
            pwmIzq = intensidad;
            pwmCtr = intensidad;
            pwmDer = intensidad;
            break;
        default:
            break;
    }

    escribirMotor(MOTOR_IZQUIERDO, LEDC_CH_IZQ, pwmIzq);
    escribirMotor(MOTOR_CENTRO,    LEDC_CH_CTR, pwmCtr);
    escribirMotor(MOTOR_DERECHO,   LEDC_CH_DER, pwmDer);
}

void Salida::escribirMotor(uint8_t pinMotor, uint8_t canalLedc, uint8_t valorPwm) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(pinMotor, valorPwm);
#else
    ledcWrite(canalLedc, valorPwm);
#endif
}
