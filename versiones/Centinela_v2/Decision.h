/**
 * @file    Decision.h
 * @brief   Módulo de lógica de decisión y síntesis de alertas — Centinela v2.0
 *
 * @details Decision es el "cerebro" del bastón. En cada ciclo del scheduler
 *          (TIEMPO_DECISION = 20 ms) lee el Estado global —que ya fue actualizado
 *          por Sensores, Radar y Movimiento— y decide:
 *
 *          1. **¿Qué nivel de alerta corresponde?** (SIN_ALERTA … EMERGENCIA)
 *          2. **¿En qué zona está el peligro?** (IZQUIERDA, CENTRO, DERECHA, TODOS)
 *          3. **¿Qué intensidad de vibración corresponde a cada motor?**
 *          4. **¿Hay que emitir un aviso de audio?**
 *
 *          El módulo escribe sus conclusiones de vuelta en Estado para que
 *          Salida y Audio las consuman en sus propios ciclos.
 *
 *          Jerarquía de prioridad (de mayor a menor):
 *          @code
 *          EMERGENCIA  ← Caída total confirmada O botón SOS presionado
 *          CRITICO     ← Cualquier sensor < DISTANCIA_PELIGRO (20 cm) O
 *                         pozo/escalón detectado por sensor de suelo
 *          PELIGRO     ← Cualquier sensor < 2×DISTANCIA_PELIGRO (40 cm) O
 *                         batería baja
 *          PRECAUCION  ← Cualquier sensor < DISTANCIA_ALERTA_MAX (80 cm)
 *          SIN_ALERTA  ← Todos los sensores > DISTANCIA_ALERTA_MAX
 *          @endcode
 *
 *          Lógica del sensor de suelo (detección de pozos):
 *          En condiciones normales de marcha, el extremo del bastón toca o está
 *          cerca del suelo (< 15 cm típicamente). Si la distancia al suelo es
 *          mayor que SUELO_UMBRAL_POZO (60 cm), hay una discontinuidad en el
 *          suelo (pozo, escalera, desnivel brusco) → CRITICO + zona CENTRO.
 *
 *          Anti-rebote de alertas de audio:
 *          Para no repetir el mismo aviso de voz cada 20 ms, Decision rastrea
 *          el nivel de alerta del ciclo anterior y el tiempo desde el último
 *          aviso. Solo emite audio cuando el nivel cambia O cuando ha pasado
 *          AUDIO_REPETICION_MS desde el último aviso.
 *
 * @note    Decision no accede directamente a los pines GPIO. Toda lectura de
 *          sensores se hace a través de Estado y toda escritura de actuadores
 *          se hace a través de Estado. Los módulos Salida y Audio son los únicos
 *          que acceden a los pines físicos.
 *
 * Hardware objetivo : ESP32-WROOM-32
 * Dependencias      : Configuracion.h, Tipos.h, Estado.h
 */

#ifndef DECISION_H
#define DECISION_H

#include <Arduino.h>
#include "Configuracion.h"
#include "Tipos.h"
#include "Estado.h"

// ============================================================================
//  CONSTANTES LOCALES DEL MÓDULO DE DECISIÓN
// ============================================================================

/// Distancia al suelo (en cm) a partir de la cual se considera un pozo o vacío.
/// En marcha normal, el bastón está a < 15 cm del suelo.
/// Si la distancia supera este umbral, hay un desnivel peligroso.
static constexpr uint16_t SUELO_UMBRAL_POZO = 60U;

/// Tiempo mínimo entre avisos de audio del mismo tipo [ms].
/// Evita saturar al usuario con la misma alerta de voz en bucle.
static constexpr uint32_t AUDIO_REPETICION_MS = 3000UL;

/// Distancia crítica de alerta doble: 2× DISTANCIA_PELIGRO.
/// Si un sensor mide menos de esto, pasamos de PRECAUCION a PELIGRO.
static constexpr uint16_t DISTANCIA_DOBLE_PELIGRO = 2U * DISTANCIA_PELIGRO; // 40 cm

/**
 * @class Decision
 * @brief Sintetiza las lecturas de todos los sensores en alertas accionables.
 *
 * Patrón módulo: todos los métodos son estáticos. No instanciar.
 *
 * Flujo de trabajo:
 * @code
 * // En setup():
 * Decision::iniciar();
 *
 * // En el scheduler cada TIEMPO_DECISION (20 ms):
 * Decision::actualizar();
 *
 * // Luego, en el mismo ciclo o en el siguiente:
 * // Salida::actualizar() lee Estado::getNivelAlerta() y Estado::getZonaPeligro()
 * // Audio::actualizar()  lee Estado::getNivelAlerta()
 * @endcode
 */
class Decision {
public:

    // =========================================================================
    //  CICLO DE VIDA
    // =========================================================================

    /**
     * @brief Inicializa el estado interno del módulo de decisión.
     *
     * Resetea el nivel de alerta previo, el timestamp del último audio
     * y los contadores de anti-rebote.
     *
     * Debe llamarse en setup() después de Estado::iniciar().
     */
    static void iniciar();

    /**
     * @brief Analiza el Estado global y publica las decisiones de alerta.
     *
     * Debe llamarse cada TIEMPO_DECISION (20 ms) desde el scheduler.
     *
     * Secuencia de ejecución:
     * 1. Leer todas las distancias y eventos desde Estado.
     * 2. Evaluar emergencias (SOS y caída) → si sí, publicar EMERGENCIA y salir.
     * 3. Evaluar nivel CRITICO (obstáculo < 20 cm o pozo).
     * 4. Evaluar nivel PELIGRO (obstáculo < 40 cm o batería baja).
     * 5. Evaluar nivel PRECAUCION (obstáculo < 80 cm).
     * 6. Si ninguno: SIN_ALERTA.
     * 7. Determinar la zona de mayor peligro.
     * 8. Publicar NivelAlerta y Zona en Estado.
     * 9. Decidir si se necesita aviso de audio (anti-rebote).
     * 10. Si el nivel bajó de EMERGENCIA a CRITICO o menos, resetear caída.
     */
    static void actualizar();

    // =========================================================================
    //  CONSULTAS
    // =========================================================================

    /**
     * @return El nivel de alerta calculado en el último ciclo.
     * (Equivalente a Estado::getNivelAlerta(), pero acceso directo sin pasar por Estado.)
     */
    static NivelAlerta getNivelAlerta();

    /**
     * @return La zona de mayor peligro en el último ciclo.
     */
    static Zona getZonaPeligro();

    /**
     * @return true si en el último ciclo se determinó que hay un pozo/escalón.
     */
    static bool hayPozo();

    /**
     * @return true si en el último ciclo se determinó una emergencia activa.
     */
    static bool hayEmergencia();

    /**
     * @return true si Decision solicita un aviso de audio en este ciclo.
     *         Audio::actualizar() debe verificar este flag y luego llamar
     *         a Decision::confirmarAudioEmitido() para evitar repetición.
     */
    static bool solicitaAudio();

    /**
     * @brief Confirma que Audio emitió el aviso solicitado.
     *
     * Resetea el flag de solicitud de audio y actualiza el timestamp
     * del último aviso para controlar la frecuencia de repetición.
     */
    static void confirmarAudioEmitido();

private:
    // Constructor privado: clase de métodos estáticos, no instanciar
    Decision() = delete;

    // ─── Estado interno ───────────────────────────────────────────────────
    static NivelAlerta _nivelPrevio;        ///< Nivel de alerta del ciclo anterior
    static Zona        _zonaPeligro;        ///< Zona de mayor peligro actual
    static bool        _pozoDetermindado;   ///< true si se detectó pozo en este ciclo
    static bool        _emergenciaActiva;   ///< true si hay emergencia en curso
    static bool        _solicitaAudio;      ///< true si se debe emitir aviso de audio
    static uint32_t    _tsUltimoAudio;      ///< Timestamp del último aviso de audio [ms]

    // ─── Métodos de análisis internos ─────────────────────────────────────

    /**
     * @brief Evalúa emergencias (SOS y caída) y actualiza Estado si corresponde.
     * @return true si hay una emergencia activa (se salta el resto del análisis).
     */
    static bool evaluarEmergencias();

    /**
     * @brief Calcula el nivel de alerta basado en las distancias y eventos.
     * @return El NivelAlerta calculado para este ciclo.
     */
    static NivelAlerta calcularNivel();

    /**
     * @brief Determina la zona de mayor peligro a partir de las distancias y el radar.
     *
     * Prioridad:
     * 1. Si caída: TODOS
     * 2. Si SOS: TODOS
     * 3. Distancia frontal < umbral: CENTRO (sensor fijo frontal)
     * 4. Pozo detectado: CENTRO (sensor suelo)
     * 5. Radar: zona del ángulo mínimo (IZQUIERDA / CENTRO / DERECHA)
     * 6. Sin peligro: NINGUNO
     *
     * @param nivel Nivel de alerta calculado previamente.
     * @return Zona de mayor peligro.
     */
    static Zona calcularZona(NivelAlerta nivel);

    /**
     * @brief Evalúa si se debe emitir un aviso de audio en este ciclo.
     *
     * Emite audio si:
     *   - El nivel de alerta cambió respecto al ciclo anterior, O
     *   - Han pasado más de AUDIO_REPETICION_MS desde el último aviso,
     *     Y el nivel actual es PELIGRO o superior.
     *
     * @param nivelActual Nivel de alerta calculado en este ciclo.
     */
    static void evaluarAudio(NivelAlerta nivelActual);
};

#endif // DECISION_H
