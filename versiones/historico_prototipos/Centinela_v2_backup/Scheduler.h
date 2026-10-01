/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Scheduler.h
 * Descripcion:
 *   Planificador cooperativo de tareas del firmware v2.
 **************************************************************************/
#ifndef SCHEDULER_H
#define SCHEDULER_H

class Scheduler {
public:
    static void iniciar();
    static void actualizar();

private:
    static unsigned long tiempoSensores;
    static unsigned long tiempoRadar;
    static unsigned long tiempoMovimiento;
    static unsigned long tiempoDecision;
    static unsigned long tiempoSalida;
    static unsigned long tiempoBluetooth;
    static unsigned long tiempoAudio;
    static unsigned long tiempoGPS;
    static unsigned long tiempoEmergencia;
    static unsigned long tiempoMicrofono;
    static unsigned long tiempoBateria;
};

#endif // SCHEDULER_H
