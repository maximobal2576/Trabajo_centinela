#ifndef TIPOS_H
#define TIPOS_H

enum Evento {
    EVENTO_NORMAL,
    EVENTO_OBSTACULO_FRONTAL,
    EVENTO_OBSTACULO_RADAR,
    EVENTO_POZO,
    EVENTO_CAIDA,
    EVENTO_EMERGENCIA,
    EVENTO_BATERIA_BAJA,
    EVENTO_ERROR
};

enum EstadoSistema {
    SISTEMA_INICIO,
    SISTEMA_NORMAL,
    SISTEMA_ALERTA,
    SISTEMA_POZO,
    SISTEMA_CAIDA,
    SISTEMA_EMERGENCIA,
    SISTEMA_DIAGNOSTICO,
    SISTEMA_ERROR
};

enum Direccion {
    DIRECCION_NINGUNA,
    DIRECCION_IZQUIERDA,
    DIRECCION_DIAGONAL_IZQUIERDA,
    DIRECCION_CENTRO,
    DIRECCION_DIAGONAL_DERECHA,
    DIRECCION_DERECHA,
    DIRECCION_FRONTAL,
    DIRECCION_SUELO
};

enum Prioridad {
    PRIORIDAD_BAJA    = 0,
    PRIORIDAD_NORMAL  = 1,
    PRIORIDAD_ALTA    = 2,
    PRIORIDAD_CRITICA = 3
};

struct DatosSensores {
    int   anguloRadar;
    int   frontal;
    int   suelo;
    int   radarIzquierda;
    int   radarCentro;
    int   radarDerecha;
    float inclinacionX;
    float inclinacionY;
    float inclinacionZ;
    bool  movimientoDetectado;
    bool  caidaDetectada;
    bool  botonSosPresionado;
};

struct DatosSalida {
    int  motorIzquierdo;
    int  motorCentro;
    int  motorDerecho;
    bool reproducirAudio;
    int  volumenI2S;
};

struct DatosSistema {
    EstadoSistema estado;
    Evento        eventoActual;
    Prioridad     prioridadActual;
    Direccion     direccion;
    int           bateria;
    bool          bluetoothConectado;
    bool          audioStreamActivo;
    unsigned long ultimoChunkAudio;
    bool          auricularesConectados; // Deteccion de jack TRRS
    bool          botonSosPresionado;
    float         gpsLatitud;
    float         gpsLongitud;
    bool          gpsFijado;
};

struct DatosApp {
    bool navegacionActiva;
    bool gpsDisponible;
    bool microfonoActivo;
    bool recorridoOfflineActivo;
};

#endif // TIPOS_H
