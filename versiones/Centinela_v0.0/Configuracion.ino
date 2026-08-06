/*
  Configuracion.ino
  Pines y variables globales
*/
#include "Variables.h"
//==============================
// SENSORES ULTRASONICOS
//==============================


// Sensor 1
// Radar con servo en la punta
const int TRIG_RADAR = 22;
const int ECHO_RADAR = 23;


// Sensor 2
// Obstaculos altura mano/frente
const int TRIG_FRONTAL = 24;
const int ECHO_FRONTAL = 25;


// Sensor 3
// Suelo / pozos / escalones
const int TRIG_SUELO = 26;
const int ECHO_SUELO = 27;



//==============================
// SERVO RADAR
//==============================

const int PIN_SERVO = 10;


//==============================
// MOTORES VIBRADORES
// Todos PWM
//==============================


// Radar izquierda
const int MOTOR_IZQUIERDO = 5;


// Radar centro
const int MOTOR_CENTRO = 6;


// Radar derecha
const int MOTOR_DERECHO = 9;


// Obstaculo frontal altura mano
const int MOTOR_FRONTAL = 11;



//==============================
// BUZZER
//==============================

const int BUZZER = 8;



//==============================
// MPU6050
// Arduino Mega I2C
//==============================

const int MPU_SDA = 20;
const int MPU_SCL = 21;



//==============================
// VARIABLES DE DISTANCIA
//==============================


long distanciaRadar = 0;

long distanciaFrontal = 0;

long distanciaSuelo = 0;



//==============================
// VARIABLES DEL RADAR
//==============================


int anguloRadar = 0;

bool radarDetenido = false;



//==============================
// VARIABLES DE SEGURIDAD
//==============================


bool caidaDetectada = false;



//==============================
// VARIABLES DE ALARMA
//==============================


// Aumento por peligro de suelo
// 50% más intensidad

int aumentoSuelo = 0;



//==============================
// CONTROL DE TIEMPO
//==============================


unsigned long tiempoActual = 0;