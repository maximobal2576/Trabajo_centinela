/**
 * @file    Salida.h
 * @brief   Módulo de actuadores hápticos (motores de vibración PWM) — Centinela v2.0
 *
 * Controla tres motores de vibración vibrando de forma proporcional y direccional
 * para guiar intuitivamente la mano del usuario ciego:
 *   • Motor Izquierdo  (GPIO 4)  → Obstáculos en sector izquierdo
 *   • Motor Central    (GPIO 5)  → Obstáculos frontales o pozos
 *   • Motor Derecho    (GPIO 18) → Obstáculos en sector derecho
 *
 * Detecta automáticamente si el core de Arduino ESP32 es v2.x o v3.x para
 * usar las funciones LEDC correspondientes.
 */

#ifndef SALIDA_H
#define SALIDA_H

#include <Arduino.h>
#include "Configuracion.h"
#include "Tipos.h"

class Salida {
public:
    /**
     * @brief Configura canales y frecuencias PWM e inicia los motores apagados.
     */
    static void iniciar();

    /**
     * @brief Evalúa el nivel de alerta y la zona de peligro desde Estado
     *        y modula la intensidad de cada motor en tiempo real.
     */
    static void actualizar();

    /**
     * @brief Apaga todos los motores inmediatamente (seguridad).
     */
    static void detenerTodos();

private:
    Salida() = delete;

    /**
     * @brief Escribe el valor PWM seguro en el pin o canal correspondiente.
     */
    static void escribirMotor(uint8_t pinMotor, uint8_t canalLedc, uint8_t valorPwm);
};

#endif // SALIDA_H
