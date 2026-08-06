/*
====================================================
CONFIGURACION.ino
Bastón Inteligente V2
====================================================
Pines, constantes y variables globales
====================================================
*/

#include <Servo.h>
#include <Wire.h>


//====================================================
// ULTRASONICOS
//====================================================

// Radar (servo)
const byte TRIG_RADAR = 22;
const byte ECHO_RADAR = 23;

// Frontal
const byte TRIG_FRONTAL = 24;
const byte ECHO_FRONTAL = 25;

// Suelo
const byte TRIG_SUELO = 26;
const byte ECHO_SUELO = 27;


//====================================================
// SERVO
//====================================================

const byte PIN_SERVO = 10;

Servo radarServo;


//====================================================
// VIBRADORES (PWM)
//====================================================

const byte MOTOR_IZQUIERDO = 5;
const byte MOTOR_CENTRO    = 6;
const byte MOTOR_DERECHO   = 9;
const byte MOTOR_FRONTAL   = 11;


//====================================================
// BUZZER
//====================================================

const byte BUZZER = 8;


//====================================================
// BLUETOOTH (Serial1)
//====================================================

// TX1 -> 18
// RX1 -> 19


//====================================================
// DFPLAYER (Serial2)
//====================================================

// TX2 -> 16
// RX2 -> 17


//====================================================
// BOTON ASISTENTE
//====================================================

const byte BOTON_VOZ = 30;


//====================================================
// DISTANCIAS
//====================================================

long distanciaRadar = 999;
long distanciaFrontal = 999;
long distanciaSuelo = 999;


//====================================================
// RADAR
//====================================================

int anguloRadar = 0;

bool radarDetenido = false;

bool direccionRadar = true;      // true = ida, false = vuelta


//====================================================
// ESTADOS
//====================================================

bool obstaculoRadar = false;
bool obstaculoFrontal = false;
bool pozoDetectado = false;

bool caidaDetectada = false;


//====================================================
// PWM DE LOS MOTORES
// (Decision.ino escribe aquí)
//====================================================

byte pwmIzquierdo = 0;
byte pwmCentro = 0;
byte pwmDerecho = 0;
byte pwmFrontal = 0;


//====================================================
// UMBRALES
//====================================================

const byte DISTANCIA_RADAR = 120;

const byte DISTANCIA_FRONTAL = 120;

const byte DISTANCIA_POZO = 30;


//====================================================
// INTENSIDADES
//====================================================

const byte PWM_MINIMO = 80;

const byte PWM_POZO = 120;

const byte PWM_MAXIMO = 255;


//====================================================
// VELOCIDAD DEL RADAR
//====================================================

const byte PASO_SERVO = 2;      // grados

const byte ANGULO_MIN = 0;

const byte ANGULO_MAX = 180;


//====================================================
// TEMPORIZADORES
//====================================================

unsigned long tRadar = 0;
unsigned long tSensores = 0;
unsigned long tMotores = 0;
unsigned long tBluetooth = 0;
unsigned long tMPU = 0;
unsigned long tDF = 0;


//====================================================
// PERIODOS (ms)
//====================================================

const byte PERIODO_RADAR = 20;

const byte PERIODO_SENSORES = 40;

const byte PERIODO_MOTORES = 20;

const int PERIODO_BT = 300;

const byte PERIODO_MPU = 50;

const byte PERIODO_DF = 20;