/*
====================================================
SENSORES.ino
Lectura de los tres HC-SR04
====================================================
*/


//====================================================
// Función genérica para medir distancia
//====================================================

long medirDistancia(byte trigPin, byte echoPin){

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  // Timeout de 25 ms
  long tiempo = pulseIn(echoPin, HIGH, 25000);

  // Si no recibió eco
  if(tiempo == 0){

    return 999;

  }

  long distancia = tiempo * 0.0343 / 2.0;

  // Filtrar valores imposibles
  if(distancia < 2 || distancia > 400){

    return 999;

  }

  return distancia;

}



//====================================================
// Inicialización
//====================================================

void iniciarSensores(){


  pinMode(TRIG_RADAR, OUTPUT);
  pinMode(ECHO_RADAR, INPUT);

  pinMode(TRIG_FRONTAL, OUTPUT);
  pinMode(ECHO_FRONTAL, INPUT);

  pinMode(TRIG_SUELO, OUTPUT);
  pinMode(ECHO_SUELO, INPUT);

}



//====================================================
// Actualizar sensores
//====================================================

void actualizarSensores(){

  if(millis() - tSensores < PERIODO_SENSORES){

    return;

  }

  tSensores = millis();


  // Sensor del radar
  distanciaRadar = medirDistancia(TRIG_RADAR, ECHO_RADAR);


  // Sensor frontal
  distanciaFrontal = medirDistancia(TRIG_FRONTAL, ECHO_FRONTAL);


  // Sensor de suelo
  distanciaSuelo = medirDistancia(TRIG_SUELO, ECHO_SUELO);

}