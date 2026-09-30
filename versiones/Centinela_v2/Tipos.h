/**
 * @file    Tipos.h
 * @brief   Definiciones de tipos, enumeraciones y estructuras compartidas — Centinela v2.0
 *
 * @details Este archivo centraliza TODOS los tipos de datos que se intercambian
 *          entre los distintos módulos del firmware (Sensores, Radar, Movimiento,
 *          Decision, Estado, Salida, Audio).
 *
 *          Al mantener los tipos en un único lugar se evita la duplicación de
 *          definiciones y se garantiza coherencia semántica en todo el proyecto.
 *
 *          Reglas de diseño:
 *          - Usar enumeraciones fuertemente tipadas (enum class) para evitar
 *            colisiones de nombres y mejorar la legibilidad.
 *          - Las estructuras son Plain Old Data (POD): sin constructores virtuales,
 *            sin herencia, fácilmente copiables entre módulos.
 *          - Todos los valores numéricos tienen unidades explícitas en el comentario.
 *
 * @note    No incluir este archivo en archivos .cpp que ya lo incluyan vía su .h.
 *          El header guard evita doble inclusión, pero es innecesario y confuso.
 *
 * Hardware objetivo : ESP32-WROOM-32 / ESP32-DevKitC
 * Lenguaje          : C++11 o superior (Arduino ESP32 core v2.x / v3.x)
 */

#ifndef TIPOS_H
#define TIPOS_H

#include <Arduino.h>   // uint8_t, uint16_t, int16_t, bool, etc.

// ============================================================================
//  ZONA DE DETECCIÓN — ¿DE QUÉ LADO VIENE EL OBSTÁCULO?
// ============================================================================

/**
 * @brief Identifica el lado del espacio donde se detectó un obstáculo.
 *
 * Usado por Radar y Decision para comunicar la posición angular del peligro
 * y por Salida para seleccionar qué motor vibrador activar.
 *
 * - NINGUNO    : no hay obstáculo en rango de alerta.
 * - IZQUIERDA  : obstáculo en el sector 45°–75° (servo).
 * - CENTRO     : obstáculo en el sector 76°–105° (servo) o sensor frontal.
 * - DERECHA    : obstáculo en el sector 106°–135° (servo).
 * - TODOS      : obstáculos múltiples o situación de emergencia total.
 */
enum class Zona : uint8_t {
    NINGUNO    = 0,  ///< Sin obstáculo detectado en rango de alerta
    IZQUIERDA  = 1,  ///< Obstáculo a la izquierda del usuario
    CENTRO     = 2,  ///< Obstáculo frontal / directo
    DERECHA    = 3,  ///< Obstáculo a la derecha del usuario
    TODOS      = 4   ///< Emergencia: múltiples sectores bloqueados
};

// ============================================================================
//  NIVEL DE ALERTA — ¿QUÉ TAN URGENTE ES EL PELIGRO?
// ============================================================================

/**
 * @brief Escala de gravedad de un peligro detectado.
 *
 * Decision::actualizar() asigna un NivelAlerta a cada ciclo y lo almacena
 * en Estado. Salida lo traduce a patrones de vibración (PWM) y Audio a frases.
 *
 * - SIN_ALERTA : distancia > DISTANCIA_ALERTA_MAX (libre de obstáculos).
 * - PRECAUCION : distancia entre DISTANCIA_ALERTA_MAX y 2×DISTANCIA_PELIGRO.
 * - PELIGRO    : distancia entre 2×DISTANCIA_PELIGRO y DISTANCIA_PELIGRO.
 * - CRITICO    : distancia ≤ DISTANCIA_PELIGRO o caída detectada.
 * - EMERGENCIA : caída confirmada + impacto O botón SOS presionado.
 */
enum class NivelAlerta : uint8_t {
    SIN_ALERTA  = 0,  ///< Camino libre, sin acción requerida
    PRECAUCION  = 1,  ///< Obstáculo lejano, vibración suave
    PELIGRO     = 2,  ///< Obstáculo cercano, vibración media-fuerte
    CRITICO     = 3,  ///< Obstáculo muy próximo o escalón/pozo
    EMERGENCIA  = 4   ///< Caída confirmada o SOS — alerta máxima inmediata
};

// ============================================================================
//  EVENTO DE MOVIMIENTO — ¿QUÉ DETECTÓ EL IMU?
// ============================================================================

/**
 * @brief Tipo de evento cinético registrado por el MPU-6050.
 *
 * Movimiento::actualizar() actualiza continuamente el estado del IMU y
 * publica el EventoMovimiento más reciente en Estado.
 *
 * - NORMAL       : aceleración dentro de rangos de caminar (0.8–1.2 G).
 * - CAIDA_LIBRE  : aceleración total < MPU_UMBRAL_CAIDA_LIBRE (está cayendo).
 * - IMPACTO      : aceleración total > MPU_UMBRAL_GOLPE (golpe brusco).
 * - CAIDA_TOTAL  : CAIDA_LIBRE seguido de IMPACTO en menos de 800 ms.
 * - INCLINADO    : bastón muy inclinado (ángulo > umbral configurado).
 */
enum class EventoMovimiento : uint8_t {
    NORMAL       = 0,  ///< Sin evento cinético relevante
    CAIDA_LIBRE  = 1,  ///< El bastón experimenta baja gravedad (cayendo)
    IMPACTO      = 2,  ///< Golpe brusco detectado por el acelerómetro
    CAIDA_TOTAL  = 3,  ///< Secuencia caída libre + impacto confirmada
    INCLINADO    = 4   ///< Bastón excesivamente inclinado (usuario agachado?)
};

// ============================================================================
//  ESTADO DEL SERVO / RADAR
// ============================================================================

/**
 * @brief Dirección del barrido angular del servo del radar.
 *
 * El radar barre de izquierda (45°) a derecha (135°) y luego vuelve.
 * Radar::actualizar() alterna entre AVANZANDO y RETROCEDIENDO.
 */
enum class DireccionRadar : uint8_t {
    AVANZANDO    = 0,  ///< El servo incrementa su ángulo (va hacia la derecha)
    RETROCEDIENDO = 1  ///< El servo decrementa su ángulo (vuelve hacia la izquierda)
};

// ============================================================================
//  LECTURA DE UN SENSOR ULTRASÓNICO
// ============================================================================

/**
 * @brief Resultado de una medición HC-SR04.
 *
 * Empaqueta la distancia medida junto con metadatos útiles para la lógica
 * de decisión. La distancia es en centímetros.
 *
 * - valida    : false si el pulso de eco fue inválido (timeout o ruido).
 * - distancia : distancia en centímetros [cm]. Ignorar si !valida.
 * - angulo    : ángulo del servo al momento de la medición (solo para el radar).
 *               Valor 0 para los sensores fijos (frontal, suelo).
 */
struct LecturaUltrasonica {
    bool     valida;      ///< true si el eco fue recibido en tiempo útil
    uint16_t distancia;   ///< Distancia medida en centímetros [cm]
    uint8_t  angulo;      ///< Ángulo del servo al medir (0 = sensor fijo) [°]
};

// ============================================================================
//  LECTURA DEL IMU (MPU-6050)
// ============================================================================

/**
 * @brief Datos crudos y procesados del MPU-6050.
 *
 * Movimiento::obtenerLectura() rellena esta estructura en cada ciclo.
 * Los ángulos son calculados por la librería MPU6050_light (complementary filter).
 *
 * - accelTotal : magnitud del vector aceleración √(ax²+ay²+az²) [G]
 * - anguloX    : inclinación en eje X (pitch, adelante/atrás) [°]
 * - anguloY    : inclinación en eje Y (roll, lateral) [°]
 * - evento     : clasificación del movimiento detectado en este ciclo
 */
struct LecturaIMU {
    float           accelTotal;   ///< Módulo de la aceleración total [G]
    float           anguloX;      ///< Ángulo de inclinación pitch [°]
    float           anguloY;      ///< Ángulo de inclinación roll [°]
    EventoMovimiento evento;      ///< Tipo de evento cinético detectado
};

// ============================================================================
//  PUNTO DEL MAPA RADAR
// ============================================================================

/**
 * @brief Un punto del barrido angular del radar: ángulo + distancia.
 *
 * Radar mantiene un arreglo de PuntoRadar (uno por posición angular).
 * Decision analiza este arreglo para identificar el sector más peligroso.
 */
struct PuntoRadar {
    uint8_t  angulo;      ///< Ángulo del servo en este punto [°]
    uint16_t distancia;   ///< Distancia medida en ese ángulo [cm]
    bool     valida;      ///< true si la medición fue exitosa
};

// ============================================================================
//  TELEMETRÍA BLUETOOTH
// ============================================================================

/**
 * @brief Paquete de telemetría enviado a la app móvil via Bluetooth SPP.
 *
 * Estado::serializar() convierte el estado global en este paquete compacto.
 * La app lo recibe como JSON o como estructura binaria (según implementación).
 *
 * - distFrontal  : distancia al obstáculo frontal [cm]
 * - distSuelo    : distancia al suelo desde el extremo del bastón [cm]
 * - distRadar    : distancia más corta medida por el radar en su barrido [cm]
 * - nivelBateria : porcentaje de carga de batería [0–100 %]
 * - nivelAlerta  : nivel de alerta actual del sistema
 * - eventoIMU    : último evento cinético detectado
 * - zonaPeligro  : zona del espacio con mayor peligro
 */
struct PaqueteTelemetria {
    uint16_t        distFrontal;   ///< Distancia sensor frontal [cm]
    uint16_t        distSuelo;     ///< Distancia sensor suelo [cm]
    uint16_t        distRadar;     ///< Distancia mínima radar [cm]
    uint8_t         nivelBateria;  ///< Porcentaje de batería [%]
    NivelAlerta     nivelAlerta;   ///< Nivel de alerta global
    EventoMovimiento eventoIMU;    ///< Evento IMU más reciente
    Zona            zonaPeligro;   ///< Zona de mayor peligro
};

// ============================================================================
//  CONSTANTES DE LÓGICA COMPARTIDAS
// ============================================================================

/// Número de posiciones angulares del radar.
/// El servo va de RADAR_IZQUIERDA (45°) a RADAR_DERECHA (135°) de a RADAR_PASO (5°).
/// Posiciones = (135 - 45) / 5 + 1 = 19
static constexpr uint8_t RADAR_NUM_POSICIONES = 19U;

/// Tiempo máximo de espera del eco ultrasónico en microsegundos.
/// A 340 m/s, 30 ms alcanza para ~5 metros de distancia (ida y vuelta).
static constexpr uint32_t ULTRASONICO_TIMEOUT_US = 30000UL;

/// Conversión de tiempo de eco (µs) a distancia (cm).
/// Velocidad del sonido ≈ 340 m/s → 34000 cm/s → 0.034 cm/µs (ida).
/// Dividir por 2 porque el pulso va y vuelve: factor = 0.017
static constexpr float ECO_A_CM = 0.017f;

/// Tiempo máximo entre caída libre e impacto para confirmar CAIDA_TOTAL [ms].
static constexpr uint32_t CAIDA_VENTANA_MS = 800UL;

/// Ángulo límite de inclinación del bastón antes de considerarlo "inclinado" [°].
static constexpr float ANGULO_MAX_INCLINACION = 45.0f;

#endif // TIPOS_H
