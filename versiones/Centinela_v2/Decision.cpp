/**
 * @file    Decision.cpp
 * @brief   Implementación del módulo de lógica de decisión — Centinela v2.0
 *
 * @details Implementa el análisis multisensor en tiempo real que determina
 *          el nivel de alerta y la zona de peligro en cada ciclo del scheduler.
 *
 *          Este módulo no toca ningún pin GPIO directamente.
 *          Lee exclusivamente de Estado y escribe exclusivamente en Estado.
 *
 *          Tabla de decisión resumida:
 *          ┌────────────────────────────────────────────────────────────────┐
 *          │ Condición                             → Nivel     │ Zona      │
 *          ├────────────────────────────────────────────────────────────────┤
 *          │ SOS presionado O CAIDA_TOTAL          → EMERGENCIA│ TODOS     │
 *          │ frontal ≤ 20 cm O radar ≤ 20 cm       → CRITICO   │ (sensor)  │
 *          │ pozo/escalón (suelo > 60 cm)          → CRITICO   │ CENTRO    │
 *          │ frontal ≤ 40 cm O radar ≤ 40 cm       → PELIGRO   │ (sensor)  │
 *          │ batería baja                          → PELIGRO   │ NINGUNO   │
 *          │ frontal ≤ 80 cm O radar ≤ 80 cm       → PRECAUCION│ (sensor)  │
 *          │ Todos los sensores > 80 cm            → SIN_ALERTA│ NINGUNO   │
 *          └────────────────────────────────────────────────────────────────┘
 *
 * Hardware objetivo : ESP32-WROOM-32
 * Dependencias      : Decision.h, Estado.h, Tipos.h, Movimiento.h, Configuracion.h
 */

#include "Decision.h"
#include "Movimiento.h"   // Para resetearCaida() tras procesar la emergencia

// ============================================================================
//  DEFINICIÓN E INICIALIZACIÓN DE VARIABLES ESTÁTICAS
// ============================================================================

NivelAlerta Decision::_nivelPrevio       = NivelAlerta::SIN_ALERTA;
Zona        Decision::_zonaPeligro       = Zona::NINGUNO;
bool        Decision::_pozoDetermindado  = false;
bool        Decision::_emergenciaActiva  = false;
bool        Decision::_solicitaAudio     = false;
uint32_t    Decision::_tsUltimoAudio     = 0UL;

// ============================================================================
//  CICLO DE VIDA
// ============================================================================

/**
 * @brief Resetea el estado interno a valores seguros.
 */
void Decision::iniciar() {
    _nivelPrevio      = NivelAlerta::SIN_ALERTA;
    _zonaPeligro      = Zona::NINGUNO;
    _pozoDetermindado = false;
    _emergenciaActiva = false;
    _solicitaAudio    = false;
    _tsUltimoAudio    = 0UL;

    // Publicar estado inicial en Estado global
    Estado::setNivelAlerta(NivelAlerta::SIN_ALERTA);
    Estado::setZonaPeligro(Zona::NINGUNO);

#if MODO_DIAGNOSTICO
    Serial.println(F("[Decision] Inicializado."));
#endif
}

/**
 * @brief Ciclo principal del módulo — llamar cada TIEMPO_DECISION (20 ms).
 *
 * Secuencia completa de análisis y publicación de resultados.
 */
void Decision::actualizar() {

    // ── Paso 1: Evaluar emergencias (máxima prioridad) ────────────────────
    // Si hay emergencia activa, se publica EMERGENCIA y se termina el análisis.
    if (evaluarEmergencias()) {
        _emergenciaActiva = true;
        evaluarAudio(NivelAlerta::EMERGENCIA);
        return;
    }

    // Si la emergencia estaba activa y ya se resolvió, resetear la caída
    if (_emergenciaActiva) {
        _emergenciaActiva = false;
        Movimiento::resetearCaida();  // Limpiar flag de caída en el módulo IMU
    }

    // ── Paso 2: Calcular el nivel de alerta basado en distancias ──────────
    NivelAlerta nivelActual = calcularNivel();

    // ── Paso 3: Determinar la zona de mayor peligro ───────────────────────
    _zonaPeligro = calcularZona(nivelActual);

    // ── Paso 4: Publicar resultados en Estado global ──────────────────────
    Estado::setNivelAlerta(nivelActual);
    Estado::setZonaPeligro(_zonaPeligro);

    // ── Paso 5: Evaluar si se necesita aviso de audio ─────────────────────
    evaluarAudio(nivelActual);

    // ── Paso 6: Guardar el nivel actual como "previo" para el próximo ciclo
    _nivelPrevio = nivelActual;
}

// ============================================================================
//  CONSULTAS PÚBLICAS
// ============================================================================

NivelAlerta Decision::getNivelAlerta() {
    return Estado::getNivelAlerta();
}

Zona Decision::getZonaPeligro() {
    return _zonaPeligro;
}

bool Decision::hayPozo() {
    return _pozoDetermindado;
}

bool Decision::hayEmergencia() {
    return _emergenciaActiva;
}

bool Decision::solicitaAudio() {
    return _solicitaAudio;
}

void Decision::confirmarAudioEmitido() {
    _solicitaAudio = false;
    _tsUltimoAudio = millis();
}

// ============================================================================
//  MÉTODOS PRIVADOS DE ANÁLISIS
// ============================================================================

/**
 * @brief Verifica si hay una emergencia activa (SOS o caída confirmada).
 *
 * Una emergencia es la condición de mayor prioridad. Cuando se detecta:
 * - Se publica NivelAlerta::EMERGENCIA en Estado.
 * - Se publica Zona::TODOS en Estado.
 * - El módulo Decision solicita un aviso de audio inmediato.
 *
 * @return true si hay emergencia activa.
 */
bool Decision::evaluarEmergencias() {
    bool sos   = Estado::getEmergenciaSOS();
    bool caida = Estado::getCaidaConfirmada();

    if (sos || caida) {
        Estado::setNivelAlerta(NivelAlerta::EMERGENCIA);
        Estado::setZonaPeligro(Zona::TODOS);
        _zonaPeligro = Zona::TODOS;

#if MODO_DIAGNOSTICO
        if (sos)   Serial.println(F("[Decision] EMERGENCIA — Botón SOS presionado!"));
        if (caida) Serial.println(F("[Decision] EMERGENCIA — Caída total confirmada!"));
#endif
        return true;
    }
    return false;
}

/**
 * @brief Calcula el NivelAlerta basado en todas las lecturas de sensores.
 *
 * Evalúa en orden de mayor a menor prioridad:
 *   CRITICO    → sensor < DISTANCIA_PELIGRO (20 cm) o pozo detectado
 *   PELIGRO    → sensor < DISTANCIA_DOBLE_PELIGRO (40 cm) o batería baja
 *   PRECAUCION → sensor < DISTANCIA_ALERTA_MAX (80 cm)
 *   SIN_ALERTA → ningún sensor en rango de alerta
 *
 * @return El NivelAlerta calculado para este ciclo.
 */
NivelAlerta Decision::calcularNivel() {
    // Leer todas las distancias desde Estado
    uint16_t distFrontal = Estado::getDistanciaFrontal();
    uint16_t distSuelo   = Estado::getDistanciaSuelo();
    uint16_t distRadar   = Estado::getDistanciaRadarMin();
    bool     frontalOK   = Estado::getFrontalValido();
    bool     sueloOK     = Estado::getSueloValido();

    // ── Detectar pozo/escalón ─────────────────────────────────────────────
    // El sensor de suelo apunta hacia abajo. En marcha normal, la distancia
    // al suelo es pequeña. Si supera SUELO_UMBRAL_POZO → hay un hueco.
    _pozoDetermindado = sueloOK && (distSuelo > SUELO_UMBRAL_POZO);

    // ── CRÍTICO: peligro inminente ────────────────────────────────────────
    // Obstáculo a menos de DISTANCIA_PELIGRO (20 cm) en cualquier sensor
    bool frontalCritico = frontalOK && (distFrontal <= DISTANCIA_PELIGRO);
    bool radarCritico   = (distRadar <= DISTANCIA_PELIGRO && distRadar < 9999U);

    if (frontalCritico || radarCritico || _pozoDetermindado) {
#if MODO_DIAGNOSTICO
        Serial.print(F("[Decision] CRITICO — F:")); Serial.print(distFrontal);
        Serial.print(F(" R:"));  Serial.print(distRadar);
        Serial.print(F(" Pozo:")); Serial.println(_pozoDetermindado ? F("SI") : F("NO"));
#endif
        return NivelAlerta::CRITICO;
    }

    // ── PELIGRO: obstáculo cercano ────────────────────────────────────────
    // Obstáculo entre DISTANCIA_PELIGRO y DISTANCIA_DOBLE_PELIGRO (20–40 cm)
    bool frontalPeligro = frontalOK && (distFrontal <= DISTANCIA_DOBLE_PELIGRO);
    bool radarPeligro   = (distRadar <= DISTANCIA_DOBLE_PELIGRO && distRadar < 9999U);
    bool bateriaBaja    = Estado::getBateriaAlarmaBaja();

    if (frontalPeligro || radarPeligro || bateriaBaja) {
        return NivelAlerta::PELIGRO;
    }

    // ── PRECAUCIÓN: obstáculo en zona de alerta temprana ─────────────────
    // Obstáculo entre DISTANCIA_DOBLE_PELIGRO y DISTANCIA_ALERTA_MAX (40–80 cm)
    bool frontalPrecaucion = frontalOK && (distFrontal <= DISTANCIA_ALERTA_MAX);
    bool radarPrecaucion   = (distRadar <= DISTANCIA_ALERTA_MAX && distRadar < 9999U);

    if (frontalPrecaucion || radarPrecaucion) {
        return NivelAlerta::PRECAUCION;
    }

    // ── SIN ALERTA: camino libre ──────────────────────────────────────────
    return NivelAlerta::SIN_ALERTA;
}

/**
 * @brief Determina la zona de mayor peligro a partir del nivel y las fuentes.
 *
 * Lógica de prioridad de zona:
 *   1. EMERGENCIA/CAÍDA                  → TODOS
 *   2. Pozo detectado por sensor suelo   → CENTRO (el pozo está delante)
 *   3. Frontal es el sensor más crítico  → CENTRO
 *   4. Radar tiene la distancia mínima   → zona según ángulo del radar
 *   5. Sin peligro                       → NINGUNO
 *
 * @param nivel El NivelAlerta calculado en este ciclo.
 * @return Zona de mayor peligro.
 */
Zona Decision::calcularZona(NivelAlerta nivel) {
    if (nivel == NivelAlerta::SIN_ALERTA) {
        return Zona::NINGUNO;
    }

    // Si hay pozo, el peligro es directo (frente al usuario)
    if (_pozoDetermindado) {
        return Zona::CENTRO;
    }

    // Comparar distancias frontal vs radar para ver cuál es más crítico
    uint16_t distFrontal = Estado::getDistanciaFrontal();
    uint16_t distRadar   = Estado::getDistanciaRadarMin();
    bool     frontalOK   = Estado::getFrontalValido();

    // Si el frontal está más cerca que el radar, el peligro es frontal
    if (frontalOK && (distFrontal <= distRadar || distRadar >= 9999U)) {
        return Zona::CENTRO;
    }

    // Si el radar tiene la distancia menor, usar la zona del radar
    Zona zonaRadar = Estado::getZonaRadar();
    if (zonaRadar != Zona::NINGUNO) {
        return zonaRadar;
    }

    // Por defecto: CENTRO (la dirección más común de peligro)
    return Zona::CENTRO;
}

/**
 * @brief Decide si emitir un aviso de audio en este ciclo.
 *
 * Criterios para emitir audio:
 *   a) El nivel de alerta subió respecto al ciclo anterior (escalada de peligro).
 *   b) El nivel sigue siendo PELIGRO o superior Y han pasado AUDIO_REPETICION_MS ms.
 *
 * Nunca se emite audio para SIN_ALERTA (para no molestar al usuario
 * cuando el camino está libre).
 *
 * @param nivelActual Nivel de alerta calculado en el ciclo actual.
 */
void Decision::evaluarAudio(NivelAlerta nivelActual) {
    // No emitir audio para SIN_ALERTA
    if (nivelActual == NivelAlerta::SIN_ALERTA) {
        _solicitaAudio = false;
        return;
    }

    uint32_t ahora = millis();

    // Criterio a: el nivel escaló (subió de nivel)
    bool nivelEscalo = (static_cast<uint8_t>(nivelActual) >
                        static_cast<uint8_t>(_nivelPrevio));

    // Criterio b: pasó suficiente tiempo desde el último aviso
    bool tiempoSuficiente = (ahora - _tsUltimoAudio) >= AUDIO_REPETICION_MS;

    // Emitir audio si se cumple cualquiera de los criterios
    if (nivelEscalo || (tiempoSuficiente && nivelActual >= NivelAlerta::PELIGRO)) {
        _solicitaAudio = true;

#if MODO_DIAGNOSTICO
        Serial.print(F("[Decision] Audio solicitado — nivel: "));
        Serial.println(static_cast<uint8_t>(nivelActual));
#endif
    }
}
