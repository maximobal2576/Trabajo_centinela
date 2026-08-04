#include "Variables.h"
void setup() {

  Serial.begin(9600);


  iniciarSensores();

  iniciarRadar();

  iniciarAlarma();

  iniciarMPU();

  iniciarBluetooth();




}



void loop() {


  //=========================
  // Lectura de sensores
  //=========================

  distanciaRadar = leerRadar();

  distanciaFrontal = leerFrontal();

  distanciaSuelo = leerSuelo();
  Serial.println("distancia");
  Serial.println(distanciaRadar);
  Serial.println(distanciaFrontal);
  Serial.println(distanciaSuelo);
  Serial.println("grados");
  Serial.println(anguloRadar);
  //=========================
  // Radar
  //=========================

  actualizarRadar();



  //=========================
  // Alarmas
  //=========================

  actualizarAlarma();



  //=========================
  // Seguridad
  //=========================

  actualizarMPU();



  //=========================
  // Comunicación
  //=========================

  actualizarBluetooth();




}