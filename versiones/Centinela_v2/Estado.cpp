/**
 * @file    Estado.cpp
 * @brief   Implementación del módulo de estado global — Centinela v2.0
 *
 * @details Aquí se definen e inicializan todas las variables estáticas declaradas
 *          en Estado.h, y se implementan los getters/setters correspondientes.
 *
 *          En C++ es obligatorio definir las variables estáticas de una clase
 *          en exactamente UN archivo .cpp. Hacerlo aquí garantiza que el linker
 *          tenga una única instancia de cada variable.
 *
 *          El valor de inicialización por defecto de cada variable ha sido elegido
 *          para el estado "más seguro" posible en el arranque del sistema:
 *          - Distancias = 9999 cm (sin obstáculo conocido).
 *          - Flags de alarma = false (sin emergencias al arrancar).
 *          - Nivel de alerta = SIN_ALERTA.
 *          - Batería = 100% (se actualizará en el primer ciclo de lectura ADC).
 *
 * Hardware objetivo : ESP32-WROOM-32
 * Dependencias      : Estado.h, Tipos.h, Configuracion.h
 */

#include "Estado.h"

// ============================================================================
//  DEFINICIÓN E INICIALIZACIÓN DE VARIABLES ESTÁTICAS
//  (Solo en este .cpp — el linker necesita exactamente una definición)
// ============================================================================

// ─── Sensores ultrasónicos — Frontal ────────────────────────────────────────
uint16_t Estado::_distFrontal   = 9999U;  ///< Sin lectura al inicio
bool     Estado::_frontalValido = false;  ///< No hay lectura válida aún

// ─── Sensores ultrasónicos — Suelo ──────────────────────────────────────────
uint16_t Estado::_distSuelo    = 9999U;  ///< Sin lectura al inicio
bool     Estado::_sueloValido  = false;  ///< No hay lectura válida aún

// ─── Radar ──────────────────────────────────────────────────────────────────
uint16_t Estado::_distRadarMin = 9999U;          ///< Sin lectura al inicio
Zona     Estado::_zonaRadar    = Zona::NINGUNO;  ///< Sin zona determinada

// El arreglo de puntos del radar se inicializa en Estado::iniciar()
PuntoRadar Estado::_puntosRadar[RADAR_NUM_POSICIONES];

// ─── IMU ────────────────────────────────────────────────────────────────────
// La estructura LecturaIMU se inicializa en Estado::iniciar() con memset
LecturaIMU Estado::_lecturaIMU         = {1.0f, 0.0f, 0.0f, EventoMovimiento::NORMAL};
bool       Estado::_caidaConfirmada    = false;

// ─── Decisión ───────────────────────────────────────────────────────────────
NivelAlerta Estado::_nivelAlerta = NivelAlerta::SIN_ALERTA;
Zona        Estado::_zonaPeligro = Zona::NINGUNO;

// ─── Batería ────────────────────────────────────────────────────────────────
uint8_t Estado::_nivelBateria      = 100U;   ///< 100% hasta la primera medición ADC
bool    Estado::_bateriaAlarmaBaja = false;

// ─── SOS ────────────────────────────────────────────────────────────────────
bool Estado::_emergenciaSOS = false;

// ─── Bluetooth ──────────────────────────────────────────────────────────────
bool Estado::_bluetoothConectado = false;

// ─── GPS ────────────────────────────────────────────────────────────────────
float Estado::_latitud  = 0.0f;
float Estado::_longitud = 0.0f;
bool  Estado::_gpsFix   = false;

// ─── Audio ──────────────────────────────────────────────────────────────────
bool Estado::_audioReproduciendo = false;

// ============================================================================
//  CICLO DE VIDA
// ============================================================================

/**
 * @brief Pone todos los campos en su valor seguro de arranque.
 *
 * Este método inicializa de forma explícita los arreglos y estructuras
 * que no pueden inicializarse con un inicializador de lista en la definición
 * de la variable estática (por ejemplo, arreglos de struct).
 */
void Estado::iniciar() {
    // Reiniciar campos simples a valores seguros
    _distFrontal        = 9999U;
    _frontalValido      = false;
    _distSuelo          = 9999U;
    _sueloValido        = false;
    _distRadarMin       = 9999U;
    _zonaRadar          = Zona::NINGUNO;
    _lecturaIMU         = {1.0f, 0.0f, 0.0f, EventoMovimiento::NORMAL};
    _caidaConfirmada    = false;
    _nivelAlerta        = NivelAlerta::SIN_ALERTA;
    _zonaPeligro        = Zona::NINGUNO;
    _nivelBateria       = 100U;
    _bateriaAlarmaBaja  = false;
    _emergenciaSOS      = false;
    _bluetoothConectado = false;
    _latitud            = 0.0f;
    _longitud           = 0.0f;
    _gpsFix             = false;
    _audioReproduciendo = false;

    // Inicializar cada punto del arreglo del radar con valores neutros
    for (uint8_t i = 0; i < RADAR_NUM_POSICIONES; i++) {
        // El ángulo inicial se calcula a partir del límite izquierdo y el paso
        _puntosRadar[i].angulo    = static_cast<uint8_t>(RADAR_IZQUIERDA + i * RADAR_PASO);
        _puntosRadar[i].distancia = 9999U;
        _puntosRadar[i].valida    = false;
    }

#if MODO_DIAGNOSTICO
    Serial.println(F("[Estado] Inicializado — todos los campos en valor seguro."));
#endif
}

/**
 * @brief Punto de actualización periódica (sin lógica propia por ahora).
 *
 * Se mantiene por coherencia de patrón módulo. En el futuro puede incluir
 * validación de watchdog: detectar si algún módulo dejó de actualizar su campo.
 */
void Estado::actualizar() {
    // Sin lógica activa. Reservado para extensiones futuras (watchdog, coherencia).
}

// ============================================================================
//  SENSORES ULTRASÓNICOS — FRONTAL
// ============================================================================

void Estado::setDistanciaFrontal(uint16_t cm) {
    _distFrontal = cm;
}

uint16_t Estado::getDistanciaFrontal() {
    return _distFrontal;
}

void Estado::setFrontalValido(bool valido) {
    _frontalValido = valido;
}

bool Estado::getFrontalValido() {
    return _frontalValido;
}

// ============================================================================
//  SENSORES ULTRASÓNICOS — SUELO
// ============================================================================

void Estado::setDistanciaSuelo(uint16_t cm) {
    _distSuelo = cm;
}

uint16_t Estado::getDistanciaSuelo() {
    return _distSuelo;
}

void Estado::setSueloValido(bool valido) {
    _sueloValido = valido;
}

bool Estado::getSueloValido() {
    return _sueloValido;
}

// ============================================================================
//  RADAR
// ============================================================================

void Estado::setDistanciaRadarMin(uint16_t cm) {
    _distRadarMin = cm;
}

uint16_t Estado::getDistanciaRadarMin() {
    return _distRadarMin;
}

void Estado::setZonaRadar(Zona zona) {
    _zonaRadar = zona;
}

Zona Estado::getZonaRadar() {
    return _zonaRadar;
}

const PuntoRadar* Estado::getPuntosRadar() {
    // Devuelve puntero de solo lectura al arreglo interno
    return _puntosRadar;
}

void Estado::setPuntoRadar(uint8_t indice, const PuntoRadar& punto) {
    if (indice < RADAR_NUM_POSICIONES) {
        _puntosRadar[indice] = punto;
    }
    // Si el índice es inválido, se ignora silenciosamente (no hay Serial aquí)
}

// ============================================================================
//  IMU — MOVIMIENTO Y CAÍDAS
// ============================================================================

void Estado::setLecturaIMU(const LecturaIMU& lectura) {
    _lecturaIMU = lectura;
}

LecturaIMU Estado::getLecturaIMU() {
    return _lecturaIMU;
}

EventoMovimiento Estado::getEventoMovimiento() {
    return _lecturaIMU.evento;
}

void Estado::setCaidaConfirmada(bool confirmada) {
    _caidaConfirmada = confirmada;
}

bool Estado::getCaidaConfirmada() {
    return _caidaConfirmada;
}

// ============================================================================
//  DECISIÓN
// ============================================================================

void Estado::setNivelAlerta(NivelAlerta nivel) {
    _nivelAlerta = nivel;
}

NivelAlerta Estado::getNivelAlerta() {
    return _nivelAlerta;
}

void Estado::setZonaPeligro(Zona zona) {
    _zonaPeligro = zona;
}

Zona Estado::getZonaPeligro() {
    return _zonaPeligro;
}

// ============================================================================
//  BATERÍA
// ============================================================================

void Estado::setNivelBateria(uint8_t porcentaje) {
    // Clampear a [0, 100] por seguridad ante errores de conversión ADC
    _nivelBateria = (porcentaje > 100U) ? 100U : porcentaje;
}

uint8_t Estado::getNivelBateria() {
    return _nivelBateria;
}

void Estado::setBateriaAlarmaBaja(bool baja) {
    _bateriaAlarmaBaja = baja;
}

bool Estado::getBateriaAlarmaBaja() {
    return _bateriaAlarmaBaja;
}

// ============================================================================
//  BOTÓN SOS / EMERGENCIA
// ============================================================================

void Estado::setEmergenciaSOS(bool activo) {
    _emergenciaSOS = activo;
}

bool Estado::getEmergenciaSOS() {
    return _emergenciaSOS;
}

// ============================================================================
//  BLUETOOTH
// ============================================================================

void Estado::setBluetoothConectado(bool conectado) {
    _bluetoothConectado = conectado;
}

bool Estado::getBluetoothConectado() {
    return _bluetoothConectado;
}

// ============================================================================
//  GPS
// ============================================================================

void Estado::setPosicionGPS(float lat, float lon) {
    _latitud  = lat;
    _longitud = lon;
}

float Estado::getLatitud() {
    return _latitud;
}

float Estado::getLongitud() {
    return _longitud;
}

void Estado::setGPSFix(bool fix) {
    _gpsFix = fix;
}

bool Estado::getGPSFix() {
    return _gpsFix;
}

// ============================================================================
//  AUDIO
// ============================================================================

void Estado::setAudioReproduciendo(bool reproduciendo) {
    _audioReproduciendo = reproduciendo;
}

bool Estado::getAudioReproduciendo() {
    return _audioReproduciendo;
}

// ============================================================================
//  TELEMETRÍA COMPUESTA
// ============================================================================

/**
 * @brief Construye un PaqueteTelemetria con snapshot del estado actual.
 *
 * Este método es llamado periódicamente por el módulo Bluetooth para
 * serializar el estado y enviarlo a la app móvil. Se realiza una copia
 * atómica de todos los campos relevantes.
 */
PaqueteTelemetria Estado::obtenerTelemetria() {
    PaqueteTelemetria pkt;
    pkt.distFrontal  = _distFrontal;
    pkt.distSuelo    = _distSuelo;
    pkt.distRadar    = _distRadarMin;
    pkt.nivelBateria = _nivelBateria;
    pkt.nivelAlerta  = _nivelAlerta;
    pkt.eventoIMU    = _lecturaIMU.evento;
    pkt.zonaPeligro  = _zonaPeligro;
    return pkt;
}

// ============================================================================
//  DIAGNÓSTICO
// ============================================================================

/**
 * @brief Imprime un resumen legible del estado del sistema por Serial USB.
 *
 * Formato de salida (ejemplo):
 * @code
 * ──── [Estado] Diagnóstico ────
 * Frontal: 45 cm  Suelo: 80 cm  Radar: 38 cm
 * Alerta : PELIGRO  Zona: CENTRO
 * Batería: 72%  Alarma: NO
 * IMU    : NORMAL  Ax=0.98G  Roll=2.1°  Pitch=-1.3°
 * GPS    : FIX  Lat=-34.6037  Lon=-58.3816
 * BT     : Conectado
 * @endcode
 */
void Estado::imprimirDiagnostico() {
#if MODO_DIAGNOSTICO
    Serial.println(F("──── [Estado] Diagnóstico ────"));

    // Distancias
    Serial.print(F("  Frontal: ")); Serial.print(_distFrontal); Serial.print(F(" cm"));
    Serial.print(F("  Suelo: "));   Serial.print(_distSuelo);   Serial.print(F(" cm"));
    Serial.print(F("  Radar: "));   Serial.print(_distRadarMin); Serial.println(F(" cm"));

    // Alerta
    Serial.print(F("  Alerta: "));
    switch (_nivelAlerta) {
        case NivelAlerta::SIN_ALERTA: Serial.print(F("SIN_ALERTA")); break;
        case NivelAlerta::PRECAUCION: Serial.print(F("PRECAUCION")); break;
        case NivelAlerta::PELIGRO:    Serial.print(F("PELIGRO"));    break;
        case NivelAlerta::CRITICO:    Serial.print(F("CRITICO"));    break;
        case NivelAlerta::EMERGENCIA: Serial.print(F("EMERGENCIA")); break;
    }
    Serial.print(F("  Zona: "));
    switch (_zonaPeligro) {
        case Zona::NINGUNO:   Serial.println(F("NINGUNO"));   break;
        case Zona::IZQUIERDA: Serial.println(F("IZQUIERDA")); break;
        case Zona::CENTRO:    Serial.println(F("CENTRO"));    break;
        case Zona::DERECHA:   Serial.println(F("DERECHA"));   break;
        case Zona::TODOS:     Serial.println(F("TODOS"));     break;
    }

    // Batería
    Serial.print(F("  Batería: ")); Serial.print(_nivelBateria); Serial.print(F("%"));
    Serial.println(_bateriaAlarmaBaja ? F("  [!BAJA]") : F("  OK"));

    // IMU
    Serial.print(F("  IMU: "));
    switch (_lecturaIMU.evento) {
        case EventoMovimiento::NORMAL:      Serial.print(F("NORMAL"));      break;
        case EventoMovimiento::CAIDA_LIBRE: Serial.print(F("CAIDA_LIBRE")); break;
        case EventoMovimiento::IMPACTO:     Serial.print(F("IMPACTO"));     break;
        case EventoMovimiento::CAIDA_TOTAL: Serial.print(F("CAIDA_TOTAL")); break;
        case EventoMovimiento::INCLINADO:   Serial.print(F("INCLINADO"));   break;
    }
    Serial.print(F("  Ax="));    Serial.print(_lecturaIMU.accelTotal, 2); Serial.print(F("G"));
    Serial.print(F("  Pitch=")); Serial.print(_lecturaIMU.anguloX, 1);    Serial.print(F("°"));
    Serial.print(F("  Roll="));  Serial.println(_lecturaIMU.anguloY, 1);

    // GPS
    Serial.print(F("  GPS: "));
    if (_gpsFix) {
        Serial.print(F("FIX  Lat="));  Serial.print(_latitud, 6);
        Serial.print(F("  Lon="));     Serial.println(_longitud, 6);
    } else {
        Serial.println(F("Sin fix"));
    }

    // Bluetooth
    Serial.println(_bluetoothConectado ? F("  BT: Conectado") : F("  BT: Desconectado"));
#endif // MODO_DIAGNOSTICO
}
