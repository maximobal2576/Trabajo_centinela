/**
 * @file    Estado.h
 * @brief   Módulo de estado global del sistema — Centinela v2.0
 *
 * @details Estado actúa como la "memoria de trabajo" centralizada del firmware.
 *          Todos los módulos (Sensores, Radar, Movimiento, Decision, Audio, Salida)
 *          escriben sus resultados aquí y leen los resultados de otros módulos
 *          desde aquí. Esto desacopla los módulos entre sí: ningún módulo
 *          llama directamente a otro módulo, sino que comparte datos a través
 *          del Estado.
 *
 *          Patrón de uso:
 *          @code
 *          // En Sensores.cpp, después de medir:
 *          Estado::setDistanciaFrontal(lectura.distancia);
 *
 *          // En Decision.cpp, para tomar decisiones:
 *          uint16_t dist = Estado::getDistanciaFrontal();
 *          @endcode
 *
 *          Concurrencia: El ESP32 utiliza un único núcleo para el loop cooperativo
 *          del scheduler, por lo que no se necesitan mutex. Si en el futuro se
 *          usa FreeRTOS con tasks en ambos núcleos, agregar SemaphoreHandle_t.
 *
 * @note    Todos los setters y getters son métodos estáticos (patrón módulo).
 *          No instanciar esta clase.
 *
 * Hardware objetivo : ESP32-WROOM-32
 * Dependencias      : Tipos.h, Configuracion.h
 */

#ifndef ESTADO_H
#define ESTADO_H

#include <Arduino.h>
#include "Tipos.h"
#include "Configuracion.h"

/**
 * @class Estado
 * @brief Repositorio centralizado de estado en tiempo real del bastón Centinela.
 *
 * Contiene todos los valores de telemetría, flags de evento y resultados
 * de procesamiento que los módulos del firmware necesitan compartir.
 * Cada campo tiene un getter y un setter estático.
 *
 * Secciones de estado:
 * - Sensores ultrasónicos (frontal, suelo, radar mínimo)
 * - IMU (aceleración, ángulos, evento de movimiento)
 * - Decisión (nivel de alerta, zona de peligro)
 * - Batería (voltaje ADC, porcentaje)
 * - Botón SOS y flags de emergencia
 * - Bluetooth (estado de conexión)
 * - GPS (latitud, longitud, fix válido)
 * - Audio (estado de reproducción)
 */
class Estado {
public:

    // =========================================================================
    //  CICLO DE VIDA DEL MÓDULO
    // =========================================================================

    /**
     * @brief Inicializa todos los campos al valor seguro por defecto.
     *
     * Debe llamarse una vez en setup() antes que cualquier otro módulo,
     * ya que los demás módulos leen Estado durante su propia inicialización.
     */
    static void iniciar();

    /**
     * @brief Punto de actualización periódica (reservado para futuras lógicas).
     *
     * Actualmente Estado es un repositorio puro (sin lógica propia).
     * Este método existe para mantener coherencia con el patrón módulo y
     * permitir agregar watchdogs o validaciones de consistencia en el futuro.
     */
    static void actualizar();

    // =========================================================================
    //  SENSORES ULTRASÓNICOS — FRONTAL
    // =========================================================================

    /** @brief Almacena la distancia medida por el sensor frontal [cm]. */
    static void setDistanciaFrontal(uint16_t cm);

    /** @return Última distancia frontal válida [cm]. 9999 si no hay lectura. */
    static uint16_t getDistanciaFrontal();

    /** @brief Indica si la última lectura frontal fue válida. */
    static void setFrontalValido(bool valido);

    /** @return true si el sensor frontal devolvió un eco válido. */
    static bool getFrontalValido();

    // =========================================================================
    //  SENSORES ULTRASÓNICOS — SUELO
    // =========================================================================

    /** @brief Almacena la distancia al suelo medida por el sensor inferior [cm]. */
    static void setDistanciaSuelo(uint16_t cm);

    /** @return Última distancia al suelo [cm]. 9999 si no hay lectura. */
    static uint16_t getDistanciaSuelo();

    /** @brief Indica si la última lectura de suelo fue válida. */
    static void setSueloValido(bool valido);

    /** @return true si el sensor de suelo devolvió eco válido. */
    static bool getSueloValido();

    // =========================================================================
    //  RADAR — DISTANCIA MÍNIMA DEL BARRIDO
    // =========================================================================

    /**
     * @brief Actualiza la distancia mínima detectada en el barrido del radar [cm].
     * @param cm  Distancia en centímetros de la detección más cercana del barrido actual.
     */
    static void setDistanciaRadarMin(uint16_t cm);

    /** @return Distancia mínima del radar en el barrido vigente [cm]. */
    static uint16_t getDistanciaRadarMin();

    /**
     * @brief Actualiza la zona (sector angular) del obstáculo más cercano del radar.
     * @param zona  IZQUIERDA, CENTRO o DERECHA según el ángulo del servo.
     */
    static void setZonaRadar(Zona zona);

    /** @return Zona angular donde el radar detectó el objeto más cercano. */
    static Zona getZonaRadar();

    /** @brief Devuelve el arreglo completo de puntos del último barrido radar. */
    static const PuntoRadar* getPuntosRadar();

    /**
     * @brief Actualiza un punto individual del mapa de barrido radar.
     * @param indice  Índice en el arreglo [0 .. RADAR_NUM_POSICIONES-1].
     * @param punto   Datos de distancia y validez para ese ángulo.
     */
    static void setPuntoRadar(uint8_t indice, const PuntoRadar& punto);

    // =========================================================================
    //  IMU — MOVIMIENTO Y CAÍDAS
    // =========================================================================

    /** @brief Almacena la lectura completa del IMU. */
    static void setLecturaIMU(const LecturaIMU& lectura);

    /** @return Copia de la última lectura del IMU. */
    static LecturaIMU getLecturaIMU();

    /** @return Último evento de movimiento detectado. */
    static EventoMovimiento getEventoMovimiento();

    /**
     * @brief Activa el flag de caída confirmada.
     * @param confirmada true al detectar secuencia caída libre + impacto.
     */
    static void setCaidaConfirmada(bool confirmada);

    /** @return true si hay una caída total confirmada pendiente de manejar. */
    static bool getCaidaConfirmada();

    // =========================================================================
    //  DECISIÓN — NIVEL DE ALERTA Y ZONA
    // =========================================================================

    /**
     * @brief Establece el nivel de alerta calculado por Decision::actualizar().
     * @param nivel Nivel de alerta (SIN_ALERTA … EMERGENCIA).
     */
    static void setNivelAlerta(NivelAlerta nivel);

    /** @return Nivel de alerta actual del sistema. */
    static NivelAlerta getNivelAlerta();

    /**
     * @brief Establece la zona de peligro dominante calculada por Decision.
     * @param zona Zona del espacio con mayor peligro.
     */
    static void setZonaPeligro(Zona zona);

    /** @return Zona de mayor peligro detectada en el ciclo actual. */
    static Zona getZonaPeligro();

    // =========================================================================
    //  BATERÍA
    // =========================================================================

    /**
     * @brief Almacena el porcentaje de batería calculado.
     * @param porcentaje Valor entre 0 y 100 [%].
     */
    static void setNivelBateria(uint8_t porcentaje);

    /** @return Porcentaje de carga de batería [%]. */
    static uint8_t getNivelBateria();

    /**
     * @brief Activa o desactiva la alarma de batería baja.
     * @param baja true cuando el nivel está por debajo de BATERIA_UMBRAL_BAJA.
     */
    static void setBateriaAlarmaBaja(bool baja);

    /** @return true si la batería está en nivel de alarma. */
    static bool getBateriaAlarmaBaja();

    // =========================================================================
    //  BOTÓN SOS / EMERGENCIA
    // =========================================================================

    /**
     * @brief Activa el flag de emergencia SOS.
     * @param activo true cuando el botón SOS fue presionado.
     */
    static void setEmergenciaSOS(bool activo);

    /** @return true si hay una emergencia SOS activa pendiente de procesar. */
    static bool getEmergenciaSOS();

    // =========================================================================
    //  BLUETOOTH
    // =========================================================================

    /**
     * @brief Actualiza el flag de conexión Bluetooth.
     * @param conectado true si hay un dispositivo emparejado y conectado.
     */
    static void setBluetoothConectado(bool conectado);

    /** @return true si el Bluetooth está conectado a la app. */
    static bool getBluetoothConectado();

    // =========================================================================
    //  GPS
    // =========================================================================

    /**
     * @brief Almacena la última posición GPS válida.
     * @param lat  Latitud en grados decimales (positivo = Norte).
     * @param lon  Longitud en grados decimales (positivo = Este).
     */
    static void setPosicionGPS(float lat, float lon);

    /** @return Última latitud registrada [°]. */
    static float getLatitud();

    /** @return Última longitud registrada [°]. */
    static float getLongitud();

    /**
     * @brief Indica si el GPS tiene un fix válido.
     * @param fix true si el NEO-6M devuelve sentencias con fix 3D.
     */
    static void setGPSFix(bool fix);

    /** @return true si el GPS tiene posición válida. */
    static bool getGPSFix();

    // =========================================================================
    //  AUDIO
    // =========================================================================

    /**
     * @brief Indica si el módulo de audio está reproduciendo en este momento.
     * @param reproduciendo true mientras haya audio en el buffer I2S.
     */
    static void setAudioReproduciendo(bool reproduciendo);

    /** @return true si el audio está activo (útil para evitar interrupciones). */
    static bool getAudioReproduciendo();

    // =========================================================================
    //  TELEMETRÍA COMPUESTA
    // =========================================================================

    /**
     * @brief Construye y devuelve un PaqueteTelemetria con el estado actual.
     *
     * Este método recopila todos los campos relevantes en una estructura compacta
     * lista para serializar y enviar por Bluetooth a la app móvil.
     *
     * @return PaqueteTelemetria con snapshot del estado en este instante.
     */
    static PaqueteTelemetria obtenerTelemetria();

    // =========================================================================
    //  DIAGNÓSTICO
    // =========================================================================

    /**
     * @brief Imprime un resumen del estado actual por el puerto Serial (USB).
     *
     * Solo activo cuando MODO_DIAGNOSTICO == true en Configuracion.h.
     * Incluye: distancias, nivel de alerta, batería, evento IMU, GPS.
     */
    static void imprimirDiagnostico();

private:
    // ─── Constructor privado: clase de métodos estáticos, no instanciar ───
    Estado() = delete;

    // ─── Campos estáticos internos ────────────────────────────────────────
    // Sensores ultrasónicos
    static uint16_t _distFrontal;
    static bool     _frontalValido;
    static uint16_t _distSuelo;
    static bool     _sueloValido;

    // Radar
    static uint16_t   _distRadarMin;
    static Zona       _zonaRadar;
    static PuntoRadar _puntosRadar[RADAR_NUM_POSICIONES];

    // IMU
    static LecturaIMU        _lecturaIMU;
    static bool              _caidaConfirmada;

    // Decisión
    static NivelAlerta _nivelAlerta;
    static Zona        _zonaPeligro;

    // Batería
    static uint8_t _nivelBateria;
    static bool    _bateriaAlarmaBaja;

    // SOS
    static bool _emergenciaSOS;

    // Bluetooth
    static bool _bluetoothConectado;

    // GPS
    static float _latitud;
    static float _longitud;
    static bool  _gpsFix;

    // Audio
    static bool _audioReproduciendo;
};

#endif // ESTADO_H
