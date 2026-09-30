 /**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Centinela_v2.ino
 * Hardware : ESP32 WROOM-32 / WROVER
 * Plataforma: Arduino core para ESP32
 *
 * Componentes integrados y validados:
 *   1. ESP32-WROOM-32 (Microcontrolador central 240MHz)
 *   2. 3x HC-SR04 (Frontal, Suelo/Desnivel, Radar angular)
 *   3. MPU-6050 (Acelerometro y Giroscopio I2C para caidas)
 *   4. Servomotor SG90 (Radar de barrido angular)
 *   5. 3x Motores vibradores de alta potencia (Feedback haptico Izq/Ctr/Der)
 *   6. Drivers MOSFET canal N (Conmutacion logica 3.3V)
 *   7. DAC de audio I2S (MAX98357A mono 3W)
 *   8. Parlante 4 Ohm / 3 W
 *   9. TRRS Breakout (Jack 3.5mm para auricular y microfono)
 *   10. Conmutacion automatica: Auricular insertado -> Mute 100% al altavoz
 *   11. Microfono para captura de voz y envio a la app
 *   12. Modulo GPS NEO-6M (Serial2 por hardware)
 *   13. Pulsador de emergencia SOS con alerta instantanea
 *   14. Portabaterias 2x 18650 + 2x baterias 18650 (7.4V - 8.4V)
 *   15. Regulador de voltaje Step-Down (5V / 3.3V) + Fusible + Interruptor
 *
 * Librerias requeridas (instalar desde Gestor de Librerias de Arduino IDE):
 *   - ESP32Servo (por Kevin Harrington)
 *   - MPU6050_light (por rfetick)
 *   (BluetoothSerial y driver/i2s.h vienen integradas con el core ESP32)
 *
 * Configuracion de placa en Arduino IDE:
 *   - Placa: "ESP32 Dev Module"
 *   - CPU Frequency: 240 MHz
 *   - Flash Size: 4MB
 *   - Upload Speed: 921600
 **************************************************************************/

#include "Configuracion.h"
#include "Tipos.h"
#include "Estado.h"
#include "Scheduler.h"

#include "Sensores.h"
#include "Radar.h"
#include "Movimiento.h"
#include "Decision.h"
#include "Salida.h"
#include "Bluetooth.h"
#include "Audio.h"
#include "GPS.h"
#include "Emergencia.h"
#include "Microfono.h"
#include "Bateria.h"
#include "Diagnostico.h"

void setup()
{
    Serial.begin(115200);

    Serial.println("======================================");
    Serial.println("CENTINELA v2.0 — SISTEMA INTEGRADO");
    Serial.println("ESP32 + I2S Audio + GPS + Microfono + TRRS");
    Serial.println("Iniciando subsistemas de hardware...");
    Serial.println("======================================");

    // 1. Estado del sistema
    Estado::iniciar();

    // 2. Sensores ultrasonicos, radar y movimiento
    Sensores::iniciar();
    Radar::iniciar();
    Movimiento::iniciar();

    // 3. Logica de control y drivers MOSFET de motores vibradores
    Decision::iniciar();
    Salida::iniciar();

    // 4. GPS NEO-6M por Hardware Serial2
    GPS::iniciar();

    // 5. Pulsador fisico de emergencia SOS
    Emergencia::iniciar();

    // 6. Microfono del baston
    Microfono::iniciar();

    // 7. Bluetooth SPP dual (comandos + audio PCM bidireccional)
    Bluetooth::iniciar();

    // 8. I2S MAX98357A y conmutador inteligente TRRS
    Audio::iniciar();

    // 9. Telemetria de bateria y diagnostico
    Bateria::iniciar();
    Diagnostico::iniciar();

    // 10. Planificador cooperativo en tiempo real
    Scheduler::iniciar();

    Estado::sistema.estado = SISTEMA_NORMAL;

    Serial.println("======================================");
    Serial.println("Centinela v2.0 Listo y Operativo.");
    Serial.println("Esperando vinculacion automatica de la app...");
    Serial.println("======================================");
}

void loop()
{
    Scheduler::actualizar();
}
