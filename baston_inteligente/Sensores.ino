/*
  Sensores.ino
  Lectura HC-SR04
*/


//================================
// Inicializar sensores
//================================

void iniciarSensores() {


  pinMode(TRIG_RADAR, OUTPUT);
  pinMode(ECHO_RADAR, INPUT);


  pinMode(TRIG_FRONTAL, OUTPUT);
  pinMode(ECHO_FRONTAL, INPUT);


  pinMode(TRIG_SUELO, OUTPUT);
  pinMode(ECHO_SUELO, INPUT);


}



//================================
// Función general de medición
//================================

long medirDistancia(const int trig,const int echo) {


  digitalWrite(trig, LOW);
  delayMicroseconds(2);


  digitalWrite(trig, HIGH);
  delayMicroseconds(10);


  digitalWrite(trig, LOW);


  long duracion = pulseIn(echo, HIGH);




  long distancia = duracion * 0.0343 / 2;



  return distancia;


}



//================================
// Sensor radar
//================================
#include "Variables.h"
long leerRadar(){

  return medirDistancia(TRIG_RADAR, ECHO_RADAR);

}



//================================
// Sensor frontal
//================================

long leerFrontal(){

  return medirDistancia(TRIG_FRONTAL, ECHO_FRONTAL);

}



//================================
// Sensor suelo
//================================

long leerSuelo(){
  return  medirDistancia(TRIG_SUELO, ECHO_SUELO);

}