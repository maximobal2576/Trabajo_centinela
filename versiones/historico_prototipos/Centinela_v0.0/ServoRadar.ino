/*
  ServoRadar.ino
  Control del servo radar
*/


#include <Servo.h>
#include "Variables.h"

Servo servoRadar;


//==============================
// Variables internas del radar
//==============================

int direccionRadar = 1;

unsigned long ultimoMovimientoRadar = 0;

const int intervaloRadar = 50; // velocidad del barrido



// Distancia para detener radar
const int DISTANCIA_RADAR = 30;



//==============================
// Inicializar radar
//==============================

void iniciarRadar(){

  servoRadar.attach(PIN_SERVO);

  anguloRadar = 0;

  servoRadar.write(anguloRadar);

}



//==============================
// Actualizar movimiento radar
//==============================

void actualizarRadar(){



  //================================
  // Verificar obstáculo
  //================================


  if(distanciaRadar > 0 && distanciaRadar <= DISTANCIA_RADAR){

    radarDetenido = true;

  }

  else{

    radarDetenido = false;

  }



  // Si hay obstáculo, no mover servo

  if(radarDetenido){

    return;

  }



  //================================
  // Movimiento sin delay
  //================================


  unsigned long ahora = millis();



  if(ahora - ultimoMovimientoRadar >= intervaloRadar){


    ultimoMovimientoRadar = ahora;



    anguloRadar += 10* direccionRadar;



    if(anguloRadar >= 180){

      anguloRadar = 180;

      direccionRadar = -1;

    }



    if(anguloRadar <= 0){

      anguloRadar = 0;

      direccionRadar = 1;

    }



    servoRadar.write(anguloRadar);


  }


}