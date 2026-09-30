/**
 * @file    Movimiento.cpp
 * @brief   Implementación del módulo de detección de movimiento y caídas — Centinela v2.0
 *
 * @details Gestiona el ciclo de lectura del MPU-6050 y la clasificación de
 *          eventos cinéticos. La detección de caída real (CAIDA_TOTAL) usa una
 *          máquina de estados de dos pasos: CAIDA_LIBRE → esperar IMPACTO.
 *
 *          Filtro complementario (en MPU6050_light):
 *          La librería combina la aceleración (para corregir la deriva a largo
 *          plazo) con el giroscopio (para respuesta rápida) usando:
 *              ángulo = α × (ángulo + ω × dt) + (1-α) × ángulo_accel
 *          donde α ≈ 0.98 por defecto.
 *
 *          Notas de rendimiento:
 *          - mpu.update() tarda aproximadamente 1–2 ms en el ESP32 a 400 kHz I2C.
 *          - Con TIEMPO_MOVIMIENTO = 20 ms, el CPU overhead es < 10%.
 *          - La detección de caída requiere al menos 2 ciclos: uno para CAIDA_LIBRE
 *            y otro para IMPACTO. Latencia mínima: 20 ms × 2 = 40 ms.
 *
 * Hardware objetivo : ESP32-WROOM-32
 * Pines             : SDA=GPIO21, SCL=GPIO22
 * Dependencias      : Movimiento.h, Wire.h, MPU6050_light.h, Estado.h
 */

#include "Movimiento.h"

// ============================================================================
//  DEFINICIÓN E INICIALIZACIÓN DE VARIABLES ESTÁTICAS
// ============================================================================

// El constructor de MPU6050 requiere una referencia a Wire.
// Se pasa Wire aquí; Wire.begin() se llama en iniciar().
MPU6050 Movimiento::_mpu(Wire);

bool             Movimiento::_mpuDisponible  = false;
float            Movimiento::_accelTotal     = 1.0f;   // 1G = reposo normal
float            Movimiento::_anguloX        = 0.0f;
float            Movimiento::_anguloY        = 0.0f;
EventoMovimiento Movimiento::_eventoActual   = EventoMovimiento::NORMAL;
bool             Movimiento::_enCaidaLibre   = false;
uint32_t         Movimiento::_tsCaidaLibre   = 0UL;

// ============================================================================
//  CICLO DE VIDA
// ============================================================================

/**
 * @brief Inicializa I2C y el MPU-6050, calibra giroscopio.
 *
 * La calibración con mpu.calcOffsets() tarda ~1 segundo mientras el bastón
 * debe estar quieto. Se imprime un mensaje para informar al operador.
 * En caso de fallo (MPU no encontrado), el sistema continúa en modo degradado.
 */
void Movimiento::iniciar() {
    // Iniciar bus I2C con los pines del mapa de hardware
    // Nota: Wire.begin() es idempotente en arduino-esp32 v2.x y v3.x
    Wire.begin(MPU_SDA, MPU_SCL);

    // Intentar inicializar el MPU-6050
    // mpu.begin() devuelve 0 si el chip responde correctamente
    byte status = _mpu.begin();

    if (status != 0) {
        // El MPU no respondió — modo degradado (sin detección de caídas)
        _mpuDisponible = false;
        Serial.print(F("[Movimiento] ERROR: MPU-6050 no encontrado. Status="));
        Serial.println(status);
        Serial.println(F("[Movimiento] Sistema sin detección de caídas."));
        return;
    }

    _mpuDisponible = true;
    Serial.println(F("[Movimiento] MPU-6050 encontrado. Calibrando giroscopio..."));
    Serial.println(F("[Movimiento] Mantener bastón quieto durante la calibración (~1 s)..."));

    // Calibración de offsets del giroscopio y acelerómetro
    // Este proceso tarda ~1 s. El bastón debe estar en reposo horizontal.
    _mpu.calcOffsets(true, true);  // (calibrar giroscopio, calibrar acelerómetro)

    Serial.println(F("[Movimiento] Calibración completada."));

    // Resetear estado de detección de caídas
    _enCaidaLibre  = false;
    _tsCaidaLibre  = 0UL;
    _eventoActual  = EventoMovimiento::NORMAL;
    _accelTotal    = 1.0f;
    _anguloX       = 0.0f;
    _anguloY       = 0.0f;
}

/**
 * @brief Lee el IMU, calcula la magnitud de aceleración y clasifica el evento.
 *
 * Si el MPU no está disponible, retorna inmediatamente sin hacer nada.
 * El periodo recomendado de llamada es TIEMPO_MOVIMIENTO = 20 ms.
 */
void Movimiento::actualizar() {
    if (!_mpuDisponible) {
        return;  // MPU no disponible — modo degradado, no hay nada que hacer
    }

    // ── Paso 1: Leer datos del sensor ─────────────────────────────────────
    // mpu.update() solicita los registros de aceleración y giroscopio al MPU
    // y actualiza los filtros complementarios internos de la librería.
    _mpu.update();

    // ── Paso 2: Obtener aceleración en los tres ejes [G] ──────────────────
    float ax = _mpu.getAccX();   // Aceleración en eje X [G]
    float ay = _mpu.getAccY();   // Aceleración en eje Y [G]
    float az = _mpu.getAccZ();   // Aceleración en eje Z [G]

    // Calcular la magnitud del vector aceleración (norma euclidiana)
    // En reposo: debería ser ~1 G (solo la gravedad)
    _accelTotal = sqrtf(ax*ax + ay*ay + az*az);

    // ── Paso 3: Obtener ángulos del filtro complementario ─────────────────
    _anguloX = _mpu.getAngleX();  // Pitch (inclinación adelante/atrás) [°]
    _anguloY = _mpu.getAngleY();  // Roll  (inclinación lateral) [°]

    // ── Paso 4: Clasificar el evento de movimiento ────────────────────────
    clasificarMovimiento();

    // ── Paso 5: Publicar la lectura en Estado ─────────────────────────────
    LecturaIMU lectura;
    lectura.accelTotal = _accelTotal;
    lectura.anguloX    = _anguloX;
    lectura.anguloY    = _anguloY;
    lectura.evento     = _eventoActual;

    Estado::setLecturaIMU(lectura);

    // Si se confirmó una caída total, activar el flag global de emergencia
    if (_eventoActual == EventoMovimiento::CAIDA_TOTAL) {
        Estado::setCaidaConfirmada(true);
    }
}

// ============================================================================
//  CONSULTAS
// ============================================================================

bool Movimiento::isMpuDisponible() {
    return _mpuDisponible;
}

float Movimiento::getAceleracionTotal() {
    return _accelTotal;
}

float Movimiento::getAnguloX() {
    return _anguloX;
}

float Movimiento::getAnguloY() {
    return _anguloY;
}

EventoMovimiento Movimiento::getEvento() {
    return _eventoActual;
}

/**
 * @brief Limpia el flag de caída confirmada en Estado y resetea la máquina de estados.
 *
 * Debe llamarse desde Decision o el módulo de emergencia una vez que la alerta
 * de caída fue procesada (vibración máxima emitida, mensaje enviado, etc.).
 */
void Movimiento::resetearCaida() {
    Estado::setCaidaConfirmada(false);
    _enCaidaLibre  = false;
    _tsCaidaLibre  = 0UL;
    if (_eventoActual == EventoMovimiento::CAIDA_TOTAL) {
        _eventoActual = EventoMovimiento::NORMAL;
    }
}

// ============================================================================
//  CLASIFICACIÓN INTERNA
// ============================================================================

/**
 * @brief Aplica la lógica de clasificación de eventos con máquina de estados.
 *
 * Orden de prioridad en la clasificación (de mayor a menor):
 *   1. CAIDA_TOTAL  : requiere secuencia CAIDA_LIBRE → IMPACTO < 800 ms
 *   2. CAIDA_LIBRE  : accelTotal < MPU_UMBRAL_CAIDA_LIBRE
 *   3. IMPACTO      : accelTotal > MPU_UMBRAL_GOLPE (fuera de ventana de caída)
 *   4. INCLINADO    : abs(anguloX) > ANGULO_MAX o abs(anguloY) > ANGULO_MAX
 *   5. NORMAL       : todos los demás casos
 */
void Movimiento::clasificarMovimiento() {
    uint32_t ahora = millis();

    // ── Caso 1: Detectar CAÍDA LIBRE ─────────────────────────────────────
    if (_accelTotal < MPU_UMBRAL_CAIDA_LIBRE) {
        if (!_enCaidaLibre) {
            // Inicio de un evento de caída libre
            _enCaidaLibre = true;
            _tsCaidaLibre = ahora;
#if MODO_DIAGNOSTICO
            Serial.println(F("[Movimiento] CAIDA_LIBRE detectada."));
#endif
        }
        _eventoActual = EventoMovimiento::CAIDA_LIBRE;
        return;
    }

    // ── Caso 2: Detectar IMPACTO y verificar secuencia de CAÍDA TOTAL ────
    if (_accelTotal > MPU_UMBRAL_GOLPE) {
        if (_enCaidaLibre) {
            // ¿El impacto ocurrió dentro de la ventana de tiempo después de la caída?
            uint32_t tiempoDesde = ahora - _tsCaidaLibre;
            if (tiempoDesde < CAIDA_VENTANA_MS) {
                // ¡Caída total confirmada! (caída libre + impacto dentro del tiempo)
                _eventoActual = EventoMovimiento::CAIDA_TOTAL;
                _enCaidaLibre = false;  // Resetear estado de caída libre
#if MODO_DIAGNOSTICO
                Serial.print(F("[Movimiento] CAIDA_TOTAL confirmada! Tiempo: "));
                Serial.print(tiempoDesde);
                Serial.println(F(" ms"));
#endif
                return;
            } else {
                // El impacto llegó demasiado tarde: era un golpe independiente
                _enCaidaLibre = false;
                _tsCaidaLibre = 0UL;
            }
        }
        // Impacto sin caída previa: golpe brusco simple
        _eventoActual = EventoMovimiento::IMPACTO;
#if MODO_DIAGNOSTICO
        Serial.print(F("[Movimiento] IMPACTO detectado. Accel="));
        Serial.print(_accelTotal, 2);
        Serial.println(F("G"));
#endif
        return;
    }

    // ── Caso 3: Verificar timeout de caída libre sin impacto ─────────────
    // Si llevamos en caída libre más de CAIDA_VENTANA_MS sin impacto,
    // fue un falso positivo (por ejemplo, el bastón fue apoyado suavemente).
    if (_enCaidaLibre && (ahora - _tsCaidaLibre) >= CAIDA_VENTANA_MS) {
        _enCaidaLibre = false;
        _tsCaidaLibre = 0UL;
#if MODO_DIAGNOSTICO
        Serial.println(F("[Movimiento] Ventana de caída expirada — no fue caída real."));
#endif
    }

    // ── Caso 4: Detectar INCLINACIÓN EXCESIVA ────────────────────────────
    // Usar valor absoluto para detectar inclinación en cualquier dirección
    float absAnguloX = fabsf(_anguloX);
    float absAnguloY = fabsf(_anguloY);

    if (absAnguloX > ANGULO_MAX_INCLINACION || absAnguloY > ANGULO_MAX_INCLINACION) {
        _eventoActual = EventoMovimiento::INCLINADO;
        return;
    }

    // ── Caso 5: Movimiento NORMAL ─────────────────────────────────────────
    _eventoActual = EventoMovimiento::NORMAL;
}
