/*
  MPU6050.ino
  Detección de caída del bastón
*/


#include <Wire.h>
#include <MPU6050.h>
#include "Variables.h"

MPU6050 mpu;


//==============================
// Variables internas
//==============================

int16_t ax, ay, az;

int16_t gx, gy, gz;


unsigned long ultimoMovimientoMPU = 0;


// Tiempo que dura la alarma de caída

const unsigned long tiempoAlarmaCaida = 5000;



//==============================
// Inicializar MPU
//==============================

void iniciarMPU(){


  Wire.begin();


  mpu.initialize();



  pinMode(BUZZER, OUTPUT);



  if(mpu.testConnection()){

    Serial.println("MPU6050 conectado");

  }

  else{

    Serial.println("Error MPU6050");

  }


}



//==============================
// Buzzer de emergencia
//==============================

void activarBuzzerCaida(){


  tone(BUZZER, 1000);


}



void apagarBuzzerCaida(){


  noTone(BUZZER);


}



//==============================
// Actualizar MPU
//==============================

void actualizarMPU(){



  mpu.getMotion6(
    &ax,
    &ay,
    &az,
    &gx,
    &gy,
    &gz
  );



  // Magnitud aproximada del movimiento

  long aceleracion = 
  abs(ax) + abs(ay) + abs(az);



  //==============================
  // Detectar golpe fuerte
  //==============================


  if(aceleracion > 50000){


    caidaDetectada = true;


    ultimoMovimientoMPU = millis();


    activarBuzzerCaida();


  }



  //==============================
  // Finalizar alerta
  //==============================


  if(caidaDetectada && 
     millis() - ultimoMovimientoMPU > tiempoAlarmaCaida){



    caidaDetectada = false;


    apagarBuzzerCaida();


  }



}