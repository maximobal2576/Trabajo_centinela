/**
 * @file    Radar.cpp
 * @brief   Implementación del módulo de radar angular — Centinela v2.0
 *
 * @details Implementa el barrido angular no bloqueante del servo SG90 + HC-SR04.
 *          Cada llamada a Radar::actualizar() avanza exactamente un ángulo en el
 *          barrido, mide la distancia y actualiza el Estado global.
 *
 *          Diseño no bloqueante:
 *          Para evitar que el servo tarde en posicionarse y bloquee el loop,
 *          se usa una máquina de estados con dos fases:
 *            - Fase 1 (MOVER): Se ordena el movimiento del servo y se registra
 *              el timestamp. Se activa _esperandoServo = true.
 *            - Fase 2 (MEDIR): En la siguiente llamada a actualizar(), si han
 *              pasado al menos 15 ms desde el movimiento, se dispara el HC-SR04
 *              y se procesa la lectura.
 *
 *          Esto garantiza que el servo tenga tiempo de llegar físicamente a la
 *          posición antes de disparar el ultrasonido, sin usar delay().
 *          (Excepción: delay(300) en iniciar() para la posición inicial.)
 *
 *          Criterio de zona angular:
 *            45° –  75° → IZQUIERDA  (sector izquierdo del usuario)
 *            76° – 105° → CENTRO     (sector frontal directo)
 *           106° – 135° → DERECHA    (sector derecho del usuario)
 *
 * Hardware objetivo : ESP32-WROOM-32
 * Pines             : TRIG=GPIO2, ECHO=GPIO36, SERVO=GPIO13
 * Dependencias      : ESP32Servo, Radar.h, Estado.h, Tipos.h, Configuracion.h
 */

#include "Radar.h"

// ============================================================================
//  DEFINICIÓN E INICIALIZACIÓN DE VARIABLES ESTÁTICAS
// ============================================================================

Servo         Radar::_servo;                               // Objeto servo (ESP32Servo)
uint8_t       Radar::_anguloActual     = RADAR_IZQUIERDA;  // Empieza en el extremo izquierdo
DireccionRadar Radar::_direccion       = DireccionRadar::AVANZANDO; // Va hacia la derecha
uint8_t       Radar::_indiceActual     = 0U;               // Primer punto del arreglo
uint16_t      Radar::_minDistBarrido   = 9999U;            // Sin mínimo hasta el primer barrido
uint8_t       Radar::_anguloMinBarrido = RADAR_CENTRO;     // Centro por defecto
uint32_t      Radar::_tsMoverServo     = 0UL;              // Timestamp del último movimiento
bool          Radar::_esperandoServo   = false;            // No está esperando aún

// ============================================================================
//  CICLO DE VIDA
// ============================================================================

/**
 * @brief Inicializa el servo y lo centra antes del primer barrido.
 *
 * Se usa delay(300) aquí porque es el único momento en que podemos bloquearnos:
 * el sistema aún no ha arrancado el scheduler y el usuario no espera respuesta.
 * El SG90 tarda aproximadamente 100–200 ms en moverse 90°; 300 ms es seguro.
 */
void Radar::iniciar() {
    // Configurar TRIG_RADAR como salida y dejar en LOW
    pinMode(TRIG_RADAR, OUTPUT);
    digitalWrite(TRIG_RADAR, LOW);

    // GPIO 36 (ECHO_RADAR) es INPUT-ONLY — no necesita pinMode explícito en ESP32,
    // pero se deja el comentario para documentar que es solo entrada
    // pinMode(ECHO_RADAR, INPUT); // ← innecesario, GPIO 34-39 siempre son INPUT

    // Asociar el servo al pin (pulsos estándar SG90: 500–2400 µs)
    // ESP32Servo usa un timer hardware → no interfiere con LEDC de los motores
    _servo.attach(PIN_SERVO, 500, 2400);

    // Llevar el servo al centro del barrido
    _servo.write(RADAR_CENTRO);

    // Esperar que el servo llegue físicamente al centro (único delay() del módulo)
    delay(300);

    // Inicializar el estado del barrido desde la posición izquierda
    _anguloActual     = RADAR_IZQUIERDA;
    _direccion        = DireccionRadar::AVANZANDO;
    _indiceActual     = 0U;
    _minDistBarrido   = 9999U;
    _anguloMinBarrido = RADAR_CENTRO;
    _esperandoServo   = false;
    _tsMoverServo     = 0UL;

    // Reiniciar distancias en Estado
    Estado::setDistanciaRadarMin(9999U);
    Estado::setZonaRadar(Zona::NINGUNO);

#if MODO_DIAGNOSTICO
    Serial.println(F("[Radar] Inicializado — servo en 90°, listo para barrer."));
#endif
}

/**
 * @brief Ejecuta un paso del barrido en la posición angular actual.
 *
 * Máquina de estados de dos fases:
 *   - Si _esperandoServo == false → ordenar movimiento del servo, activar espera.
 *   - Si _esperandoServo == true  → verificar si pasaron 15 ms; si sí, medir y avanzar.
 *
 * Llamar desde el scheduler con período TIEMPO_RADAR (60 ms).
 */
void Radar::actualizar() {
    uint32_t ahora = millis();

    // ── FASE 1: Mover servo a la posición actual del barrido ──────────────
    if (!_esperandoServo) {
        _servo.write(_anguloActual);          // Ordena el movimiento
        _tsMoverServo  = ahora;               // Guarda el momento del movimiento
        _esperandoServo = true;               // Pasar a fase de espera
        return;                               // Salir: medir en la siguiente llamada
    }

    // ── FASE 2: Verificar tiempo de asentamiento del servo ────────────────
    // El SG90 necesita aproximadamente 10–15 ms para avanzar 5° (un paso).
    // Esperamos 15 ms antes de disparar el ultrasonido.
    if ((ahora - _tsMoverServo) < 15UL) {
        return;  // Servo aún en movimiento — salir sin medir
    }

    // ── FASE 2 (continuación): Medir distancia en la posición actual ──────
    _esperandoServo = false;  // Reset para el próximo ciclo

    LecturaUltrasonica lectura = medirDistancia();

    // Construir el punto del mapa radar
    PuntoRadar punto;
    punto.angulo    = _anguloActual;
    punto.distancia = lectura.valida ? lectura.distancia : 9999U;
    punto.valida    = lectura.valida;

    // Almacenar el punto en Estado (índice calculado desde el ángulo)
    Estado::setPuntoRadar(_indiceActual, punto);

    // Actualizar el mínimo de distancia en este barrido
    if (lectura.valida && lectura.distancia < _minDistBarrido) {
        _minDistBarrido   = lectura.distancia;
        _anguloMinBarrido = _anguloActual;
    }

    // Publicar el mínimo actual en Estado (se actualiza a medida que avanza el barrido)
    Estado::setDistanciaRadarMin(_minDistBarrido);
    Estado::setZonaRadar(anguloAZona(_anguloMinBarrido));

#if MODO_DIAGNOSTICO
    // Imprimir resultado de este punto (solo cuando hay lectura válida)
    if (lectura.valida) {
        Serial.print(F("[Radar] "));
        Serial.print(_anguloActual);
        Serial.print(F("° → "));
        Serial.print(lectura.distancia);
        Serial.println(F(" cm"));
    }
#endif

    // ── Avanzar al siguiente ángulo ───────────────────────────────────────
    if (_direccion == DireccionRadar::AVANZANDO) {
        // Movimiento hacia la derecha (ángulo creciente)
        if (_anguloActual >= RADAR_DERECHA) {
            // Llegamos al extremo derecho: invertir dirección
            _direccion = DireccionRadar::RETROCEDIENDO;

            // Barrido completo: reiniciar el mínimo para el próximo barrido
            _minDistBarrido   = 9999U;
            _anguloMinBarrido = RADAR_CENTRO;

#if MODO_DIAGNOSTICO
            Serial.println(F("[Radar] Extremo derecho — invirtiendo dirección."));
#endif
        } else {
            _anguloActual += RADAR_PASO;
            _indiceActual++;
            // Clampear índice por seguridad
            if (_indiceActual >= RADAR_NUM_POSICIONES) {
                _indiceActual = RADAR_NUM_POSICIONES - 1U;
            }
        }
    } else {
        // Movimiento hacia la izquierda (ángulo decreciente)
        if (_anguloActual <= RADAR_IZQUIERDA) {
            // Llegamos al extremo izquierdo: invertir dirección
            _direccion = DireccionRadar::AVANZANDO;

            // Barrido completo: reiniciar el mínimo para el próximo barrido
            _minDistBarrido   = 9999U;
            _anguloMinBarrido = RADAR_CENTRO;

#if MODO_DIAGNOSTICO
            Serial.println(F("[Radar] Extremo izquierdo — invirtiendo dirección."));
#endif
        } else {
            _anguloActual -= RADAR_PASO;
            // Calcular el índice correcto según el ángulo actual
            // índice = (ángulo - RADAR_IZQUIERDA) / RADAR_PASO
            _indiceActual = static_cast<uint8_t>((_anguloActual - RADAR_IZQUIERDA) / RADAR_PASO);
        }
    }
}

// ============================================================================
//  CONSULTAS DE ESTADO INTERNO
// ============================================================================

uint8_t Radar::getAnguloActual() {
    return _anguloActual;
}

DireccionRadar Radar::getDireccion() {
    return _direccion;
}

/**
 * @brief Clasifica un ángulo del servo en una zona del espacio.
 *
 * Criterio:
 *  45° –  75° → IZQUIERDA  (0° a 30° desde el extremo izquierdo)
 *  76° – 105° → CENTRO     (31° a 60° desde el extremo izquierdo, cruzando 90°)
 * 106° – 135° → DERECHA    (61° a 90° desde el extremo izquierdo)
 */
Zona Radar::anguloAZona(uint8_t angulo) {
    if (angulo <= 75U)  return Zona::IZQUIERDA;
    if (angulo <= 105U) return Zona::CENTRO;
    return Zona::DERECHA;
}

/**
 * @brief Mueve el servo inmediatamente a un ángulo específico.
 *
 * Clampeado al rango [RADAR_IZQUIERDA, RADAR_DERECHA] para proteger
 * mecánicamente el servo de posiciones fuera de rango.
 */
void Radar::irAAngulo(uint8_t angulo) {
    // Clampear al rango físico del barrido
    if (angulo < RADAR_IZQUIERDA) angulo = RADAR_IZQUIERDA;
    if (angulo > RADAR_DERECHA)   angulo = RADAR_DERECHA;
    _servo.write(angulo);
}

// ============================================================================
//  MEDICIÓN ULTRASÓNICA INTERNA
// ============================================================================

/**
 * @brief Genera un pulso TRIG de 10 µs y mide la duración del eco.
 *
 * Protocolo HC-SR04:
 *   1. Asegurar que TRIG está en LOW por al menos 2 µs.
 *   2. Subir TRIG a HIGH por exactamente 10 µs.
 *   3. Bajar TRIG a LOW.
 *   4. Esperar el pulso de eco en ECHO (pulseIn con timeout).
 *   5. Convertir duración (µs) a distancia (cm): d = t × ECO_A_CM.
 *
 * Validación:
 *   - Si pulseIn devuelve 0 → timeout → lectura inválida.
 *   - Si la distancia calculada supera 400 cm → fuera del rango útil del HC-SR04 → inválida.
 *   - Rango útil del HC-SR04: 2 cm – 400 cm.
 *
 * @return LecturaUltrasonica con distancia en cm y flag de validez.
 */
LecturaUltrasonica Radar::medirDistancia() {
    LecturaUltrasonica resultado;
    resultado.valida    = false;
    resultado.distancia = 9999U;
    resultado.angulo    = _anguloActual;

    // Paso 1: Asegurar LOW mínimo de 2 µs
    digitalWrite(TRIG_RADAR, LOW);
    delayMicroseconds(2);

    // Paso 2: Pulso TRIG de 10 µs
    digitalWrite(TRIG_RADAR, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_RADAR, LOW);

    // Paso 3: Medir duración del eco (máximo ULTRASONICO_TIMEOUT_US = 30000 µs)
    uint32_t duracion = pulseIn(ECHO_RADAR, HIGH, ULTRASONICO_TIMEOUT_US);

    // Paso 4: Validar y convertir
    if (duracion == 0UL) {
        // Timeout: el eco no llegó → objeto demasiado lejos o fuera de eje
        return resultado;
    }

    float distanciaCm = static_cast<float>(duracion) * ECO_A_CM;

    // Rango útil del HC-SR04: 2 cm – 400 cm
    if (distanciaCm < 2.0f || distanciaCm > 400.0f) {
        return resultado;
    }

    resultado.valida    = true;
    resultado.distancia = static_cast<uint16_t>(distanciaCm);
    return resultado;
}
