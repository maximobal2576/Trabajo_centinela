/**
 * @file    Radar.h
 * @brief   Módulo de radar angular servo + HC-SR04 — Centinela v2.0
 *
 * @details El radar es el componente de detección direccional del bastón.
 *          Combina un servo motor SG90 con un sensor ultrasónico HC-SR04
 *          montado sobre su eje para barrer el espacio frente al usuario
 *          en un arco de 45° a 135° (90° de cobertura total).
 *
 *          Funcionamiento general:
 *          1. El servo se posiciona en un ángulo determinado (ángulo actual).
 *          2. Se dispara un pulso TRIG de 10 µs en GPIO 2.
 *          3. Se mide la duración del eco en GPIO 36 (INPUT-ONLY, ADC1_CH0).
 *          4. La distancia se calcula con la fórmula: d = t_eco × 0.017 [cm].
 *          5. El resultado se almacena en Estado (punto del mapa y distancia mínima).
 *          6. Se avanza al siguiente ángulo (±RADAR_PASO grados).
 *          7. Al llegar a los extremos (45° ó 135°), se invierte la dirección.
 *
 *          Concepto de "barrido en tiempo real":
 *          Cada llamada a Radar::actualizar() procesa SOLO UNA posición angular.
 *          Esto evita bloquear el loop principal con múltiples pulsos ultrasónicos
 *          consecutivos. Un barrido completo tarda: 19 posiciones × TIEMPO_RADAR ms.
 *
 *          Con TIEMPO_RADAR = 60 ms → un barrido completo cada ~1140 ms ≈ 1.1 s.
 *
 *          Cálculo de posiciones angulares:
 *          - Mínimo: RADAR_IZQUIERDA = 45°
 *          - Máximo: RADAR_DERECHA   = 135°
 *          - Paso:   RADAR_PASO      = 5°
 *          - Total:  (135-45)/5 + 1  = 19 posiciones
 *
 * @warning El pin ECHO_RADAR (GPIO 36) es INPUT-ONLY en el ESP32.
 *          No tiene pull-up interno. Conectar un pull-down de 10 kΩ externamente
 *          para evitar que el pin flote cuando el sensor no envía eco.
 *
 * @warning El pin TRIG_RADAR (GPIO 2) tiene pull-down externo recomendado de 10 kΩ
 *          para evitar que el ESP32 entre en modo de descarga al bootear.
 *
 * @note    Se usa la librería ESP32Servo (por Kevin Harrington) que usa el
 *          timer hardware del ESP32 para generar la señal PWM del servo,
 *          evitando conflictos con el LEDC de los motores de vibración.
 *
 * Hardware objetivo : ESP32-WROOM-32
 * Dependencias      : ESP32Servo, Configuracion.h, Tipos.h, Estado.h
 */

#ifndef RADAR_H
#define RADAR_H

#include <Arduino.h>
#include <ESP32Servo.h>     // Librería de servo para ESP32 (no usar Servo.h estándar)
#include "Configuracion.h"
#include "Tipos.h"
#include "Estado.h"

/**
 * @class Radar
 * @brief Controla el barrido angular servo+HC-SR04 y actualiza el Estado global.
 *
 * Patrón módulo: todos los métodos son estáticos, no se instancia la clase.
 * El estado interno del módulo (ángulo actual, dirección de barrido) se mantiene
 * en variables estáticas privadas definidas en Radar.cpp.
 */
class Radar {
public:

    // =========================================================================
    //  CICLO DE VIDA
    // =========================================================================

    /**
     * @brief Inicializa el servo y lo posiciona en el centro del barrido.
     *
     * Acciones:
     * 1. Llama a ESP32Servo::attach() en PIN_SERVO (GPIO 13) con los
     *    parámetros de pulso estándar para el SG90 (500–2400 µs).
     * 2. Mueve el servo a RADAR_CENTRO (90°).
     * 3. Espera 300 ms (único delay() permitido en este módulo) para que el
     *    servo llegue físicamente al centro antes del primer barrido.
     * 4. Inicializa el índice de barrido al ángulo izquierdo (45°).
     * 5. Reinicia la distancia mínima en Estado a 9999 cm.
     *
     * Debe llamarse una sola vez en setup(), después de Estado::iniciar().
     */
    static void iniciar();

    /**
     * @brief Ejecuta UN paso del barrido en la posición angular actual.
     *
     * Debe llamarse periódicamente desde el scheduler con período TIEMPO_RADAR.
     *
     * Acciones por llamada:
     * 1. Mover el servo al ángulo actual (_anguloActual).
     * 2. Esperar 15 ms (tiempo de asentamiento del servo, sin delay → usando
     *    millis() en modo no bloqueante).
     * 3. Disparar pulso TRIG de 10 µs.
     * 4. Medir duración del eco (pulseIn con timeout).
     * 5. Convertir a distancia [cm].
     * 6. Actualizar el PuntoRadar correspondiente en Estado.
     * 7. Si la distancia es la mínima del barrido, actualizar distancia mínima y zona.
     * 8. Avanzar el ángulo en ±RADAR_PASO y cambiar dirección al llegar al extremo.
     * 9. Al completar un barrido completo (llegar a un extremo), reiniciar el mínimo.
     */
    static void actualizar();

    // =========================================================================
    //  CONSULTAS DE ESTADO INTERNO
    // =========================================================================

    /** @return Ángulo actual del servo en grados [°]. */
    static uint8_t getAnguloActual();

    /** @return Dirección actual del barrido (AVANZANDO o RETROCEDIENDO). */
    static DireccionRadar getDireccion();

    /**
     * @brief Devuelve la zona (IZQUIERDA / CENTRO / DERECHA) para un ángulo dado.
     *
     * La clasificación es:
     * - 45° – 75°   → IZQUIERDA
     * - 76° – 105°  → CENTRO
     * - 106° – 135° → DERECHA
     *
     * @param angulo Ángulo del servo en grados [°].
     * @return Zona correspondiente al ángulo.
     */
    static Zona anguloAZona(uint8_t angulo);

    /**
     * @brief Fuerza el servo a un ángulo específico (uso en diagnóstico).
     *
     * @param angulo Ángulo destino en grados [45–135]. Se clampeará al rango.
     * @warning No usar durante el barrido normal, ya que Radar::actualizar()
     *          sobreescribirá el ángulo en el siguiente ciclo.
     */
    static void irAAngulo(uint8_t angulo);

private:
    // Constructor privado: clase de métodos estáticos, no instanciar
    Radar() = delete;

    // ─── Objeto servo (singleton) ─────────────────────────────────────────
    static Servo _servo;

    // ─── Estado del barrido ───────────────────────────────────────────────
    static uint8_t        _anguloActual;     ///< Ángulo actual del servo [°]
    static DireccionRadar _direccion;        ///< Sentido actual del barrido
    static uint8_t        _indiceActual;     ///< Índice en el arreglo de puntos [0..18]
    static uint16_t       _minDistBarrido;   ///< Mínimo de distancia en el barrido actual [cm]
    static uint8_t        _anguloMinBarrido; ///< Ángulo donde se detectó el mínimo [°]
    static uint32_t       _tsMoverServo;     ///< Timestamp del último movimiento de servo [ms]
    static bool           _esperandoServo;   ///< true mientras se espera que el servo se asiente

    // ─── Método interno de medición ──────────────────────────────────────

    /**
     * @brief Dispara el HC-SR04 y retorna una LecturaUltrasonica.
     *
     * Genera un pulso TRIG de 10 µs en TRIG_RADAR y mide la duración
     * del eco en ECHO_RADAR con pulseIn(). El timeout es ULTRASONICO_TIMEOUT_US.
     *
     * @return LecturaUltrasonica con la distancia en cm y flag de validez.
     */
    static LecturaUltrasonica medirDistancia();
};

#endif // RADAR_H
