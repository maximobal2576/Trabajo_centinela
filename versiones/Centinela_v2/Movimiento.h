/**
 * @file    Movimiento.h
 * @brief   Módulo de detección de movimiento y caídas — IMU MPU-6050 — Centinela v2.0
 *
 * @details Este módulo gestiona el acelerómetro/giroscopio MPU-6050 conectado
 *          por I2C (SDA=GPIO21, SCL=GPIO22). Lee los datos de inercia del bastón
 *          y los clasifica en eventos de movimiento que el módulo Decision
 *          utiliza para generar alertas.
 *
 *          Tipos de eventos detectados:
 *          - NORMAL      : El bastón se mueve normalmente al caminar (0.8–1.2 G).
 *          - CAIDA_LIBRE : La aceleración total cae por debajo de MPU_UMBRAL_CAIDA_LIBRE
 *                          (≈ 0.45 G), indicando que el bastón —o el usuario— están
 *                          cayendo.
 *          - IMPACTO     : La aceleración total supera MPU_UMBRAL_GOLPE (1.75 G),
 *                          indicando un golpe brusco contra el suelo u obstáculo.
 *          - CAIDA_TOTAL : Secuencia CAIDA_LIBRE seguida de IMPACTO dentro de
 *                          CAIDA_VENTANA_MS (800 ms) → caída real del usuario.
 *          - INCLINADO   : El ángulo calculado por el filtro complementario
 *                          supera ANGULO_MAX_INCLINACION (45°) → bastón muy inclinado.
 *
 *          Algoritmo de detección de caída total:
 *          @code
 *          t0 = millis() cuando accelTotal < UMBRAL_CAIDA_LIBRE
 *          Si luego accelTotal > UMBRAL_GOLPE Y (millis() - t0) < CAIDA_VENTANA_MS:
 *              → CAIDA_TOTAL confirmada
 *          Si (millis() - t0) >= CAIDA_VENTANA_MS sin impacto:
 *              → No es caída real (falso positivo), resetear t0
 *          @endcode
 *
 *          La librería MPU6050_light calcula los ángulos usando un filtro
 *          complementario con la aceleración y el giroscopio, sin necesidad
 *          de DMP (Digital Motion Processor). Esto simplifica la inicialización
 *          y reduce la dependencia de firmware del chip.
 *
 * @note    El MPU-6050 opera con I2C a 400 kHz (Fast Mode). La dirección I2C
 *          por defecto es 0x68 (pin AD0 = GND). Si AD0 = VCC → dirección 0x69.
 *
 * @warning Si el MPU-6050 no responde al arranque, el módulo reporta el error
 *          por Serial y continúa con el sistema funcionando en modo degradado
 *          (sin detección de caídas). No bloquea el setup().
 *
 * Hardware objetivo : ESP32-WROOM-32
 * Pines             : SDA=GPIO21, SCL=GPIO22
 * Librería          : MPU6050_light por rfetick (Gestor de Librerías Arduino)
 * Dependencias      : Wire.h, MPU6050_light.h, Configuracion.h, Tipos.h, Estado.h
 */

#ifndef MOVIMIENTO_H
#define MOVIMIENTO_H

#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_light.h>    // Librería liviana para MPU-6050 (sin DMP)
#include "Configuracion.h"
#include "Tipos.h"
#include "Estado.h"

/**
 * @class Movimiento
 * @brief Gestiona el IMU MPU-6050 y detecta eventos cinéticos del bastón.
 *
 * Patrón módulo: todos los métodos son estáticos. No instanciar.
 *
 * Flujo de trabajo en el scheduler:
 * @code
 * // En setup():
 * Wire.begin(MPU_SDA, MPU_SCL);
 * Movimiento::iniciar();
 *
 * // En loop() / scheduler cada TIEMPO_MOVIMIENTO (20 ms):
 * Movimiento::actualizar();
 * @endcode
 */
class Movimiento {
public:

    // =========================================================================
    //  CICLO DE VIDA
    // =========================================================================

    /**
     * @brief Inicializa el bus I2C y el sensor MPU-6050.
     *
     * Acciones:
     * 1. Llama a Wire.begin(MPU_SDA, MPU_SCL) para iniciar el bus I2C.
     *    (Si el bus ya fue iniciado por otro módulo, Wire.begin() es idempotente.)
     * 2. Inicializa el objeto MPU6050_light y verifica comunicación.
     * 3. Si el MPU no responde: imprime error, activa _mpuDisponible = false.
     * 4. Si el MPU responde: calibra el offset de giroscopio (≈ 1 s sin mover).
     * 5. Resetea el estado de detección de caídas.
     *
     * @note La calibración requiere que el bastón esté quieto al arrancar.
     *       El tiempo de calibración de MPU6050_light es aproximadamente 1 segundo.
     */
    static void iniciar();

    /**
     * @brief Lee el IMU, clasifica el movimiento y actualiza Estado.
     *
     * Debe llamarse desde el scheduler con período TIEMPO_MOVIMIENTO (20 ms).
     * Con 20 ms entre llamadas, la tasa de muestreo efectiva es 50 Hz.
     *
     * Acciones:
     * 1. Leer aceleración y ángulos con mpu.update().
     * 2. Calcular la magnitud de aceleración total.
     * 3. Clasificar en EventoMovimiento según los umbrales.
     * 4. Actualizar la máquina de estados de CAIDA_TOTAL.
     * 5. Publicar LecturaIMU en Estado.
     * 6. Si hay CAIDA_TOTAL: activar Estado::setCaidaConfirmada(true).
     *
     * Si _mpuDisponible == false, este método retorna sin hacer nada.
     */
    static void actualizar();

    // =========================================================================
    //  CONSULTAS
    // =========================================================================

    /**
     * @return true si el MPU-6050 fue inicializado correctamente.
     *         false si el chip no respondió al arranque (modo degradado).
     */
    static bool isMpuDisponible();

    /**
     * @return La aceleración total calculada en el último ciclo [G].
     *         (magnitud del vector: √(ax²+ay²+az²) )
     */
    static float getAceleracionTotal();

    /**
     * @return Ángulo de inclinación X (pitch) en el último ciclo [°].
     */
    static float getAnguloX();

    /**
     * @return Ángulo de inclinación Y (roll) en el último ciclo [°].
     */
    static float getAnguloY();

    /**
     * @return Último EventoMovimiento clasificado.
     */
    static EventoMovimiento getEvento();

    /**
     * @brief Resetea el flag de caída confirmada (llamar después de procesar la emergencia).
     *
     * Decision o el módulo de emergencia deben llamar a este método después de
     * haber procesado la alerta de caída, para evitar re-disparar la emergencia.
     */
    static void resetearCaida();

private:
    // Constructor privado: clase de métodos estáticos, no instanciar
    Movimiento() = delete;

    // ─── Objeto IMU ───────────────────────────────────────────────────────
    static MPU6050 _mpu;

    // ─── Flags de estado ─────────────────────────────────────────────────
    static bool _mpuDisponible;        ///< true si el MPU respondió en iniciar()

    // ─── Lectura actual ───────────────────────────────────────────────────
    static float            _accelTotal;  ///< Magnitud de aceleración [G]
    static float            _anguloX;     ///< Ángulo pitch [°]
    static float            _anguloY;     ///< Ángulo roll [°]
    static EventoMovimiento _eventoActual; ///< Clasificación del movimiento actual

    // ─── Máquina de estados de caída ─────────────────────────────────────
    static bool     _enCaidaLibre;        ///< true mientras se detecta baja G
    static uint32_t _tsCaidaLibre;        ///< Timestamp del inicio de la caída libre [ms]

    // ─── Método interno de clasificación ─────────────────────────────────

    /**
     * @brief Clasifica el movimiento basándose en _accelTotal y los ángulos.
     *
     * Aplica la lógica de umbrales y la máquina de estados de CAIDA_TOTAL.
     * Actualiza _eventoActual y, si corresponde, llama a Estado::setCaidaConfirmada(true).
     */
    static void clasificarMovimiento();
};

#endif // MOVIMIENTO_H
