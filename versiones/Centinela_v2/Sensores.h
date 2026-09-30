/**
 * @file    Sensores.h
 * @brief   Subsistema de sensores ultrasónicos HC-SR04 (frontal y suelo).
 *
 * Encapsula la lógica de hardware para los dos sensores fijos del bastón.
 * Los resultados de cada medición se almacenan en Estado::sensores.
 *
 * ─── DISEÑO ─────────────────────────────────────────────────────────────────
 *  ┌─ leerDistancia() se declara PUBLIC para que Radar.cpp pueda reutilizarla
 *  │   sin duplicar la lógica de disparo/medición ultrasónica.
 *  └─ Patrón de clase estática: todos los miembros son static, sin instancias.
 * ────────────────────────────────────────────────────────────────────────────
 *
 * HARDWARE:
 *   • HC-SR04 frontal  → detecta obstáculos a la altura del torso/cabeza
 *   • HC-SR04 suelo    → detecta pozos, escaleras y desniveles en el piso
 *
 * PINES (definidos en Configuracion.h):
 *   TRIG_FRONTAL, ECHO_FRONTAL, TRIG_SUELO, ECHO_SUELO
 */

#ifndef SENSORES_H
#define SENSORES_H

#include <Arduino.h>

/**
 * @class Sensores
 * @brief Módulo de adquisición de datos ultrasónicos (sensores fijos).
 */
class Sensores {
public:
    // ═══════════════════════════════════════════════════════════
    // INTERFAZ PÚBLICA
    // ═══════════════════════════════════════════════════════════

    /**
     * @brief Inicializa los pines GPIO de todos los sensores ultrasónicos.
     *
     * Configura TRIG como OUTPUT y ECHO como INPUT para cada sensor.
     * Deja los pines TRIG en LOW para evitar pulsos erráticos al encender.
     * Llamar una sola vez desde setup() o Sensores::iniciar().
     */
    static void iniciar();

    /**
     * @brief Lee los sensores frontal y suelo, y actualiza Estado::sensores.
     *
     * Se invoca periódicamente desde el Scheduler con período TIEMPO_SENSORES.
     * Actualiza:
     *   • Estado::sensores.frontal → distancia al obstáculo frontal (cm)
     *   • Estado::sensores.suelo  → distancia al piso (cm)
     */
    static void actualizar();

    /**
     * @brief Mide la distancia con un sensor HC-SR04 genérico (reutilizable).
     *
     * Función PUBLIC para que otros módulos (ej. Radar) puedan medir distancias
     * con sus propios pines sin duplicar el protocolo de disparo ultrasónico.
     *
     * Protocolo HC-SR04:
     *   1. Pulso LOW de 2 µs en TRIG (limpia señal residual)
     *   2. Pulso HIGH de 10 µs en TRIG (disparo del ultrasonido)
     *   3. Medir duración del pulso HIGH en ECHO (tiempo de vuelo)
     *   4. Distancia = duración × 0.034 / 2  [cm]
     *
     * @param trig  Pin TRIGGER conectado al sensor (OUTPUT del ESP32).
     * @param echo  Pin ECHO    conectado al sensor (INPUT  del ESP32).
     * @return Distancia en cm. Devuelve 400 si hay timeout (espacio libre ≥ 2.5 m).
     *         Rango válido del sensor: 2 – 400 cm.
     */
    static int leerDistancia(int trig, int echo);
};

#endif // SENSORES_H
