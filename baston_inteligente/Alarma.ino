/*
  Alarma.ino
  Control de motores vibradores
*/


//==============================
// Variables internas
//==============================
#include "Variables.h"

unsigned long ultimoPulso = 0;


//==============================
// Inicializar alarma
//==============================

void iniciarAlarma(){


  pinMode(MOTOR_IZQUIERDO, OUTPUT);

  pinMode(MOTOR_CENTRO, OUTPUT);

  pinMode(MOTOR_DERECHO, OUTPUT);

  pinMode(MOTOR_FRONTAL, OUTPUT);

  pinMode(BUZZER, OUTPUT);


  apagarMotores();

}



//==============================
// Apagar todos
//==============================

void apagarMotores(){


  analogWrite(MOTOR_IZQUIERDO,0);

  analogWrite(MOTOR_CENTRO,0);

  analogWrite(MOTOR_DERECHO,0);

  analogWrite(MOTOR_FRONTAL,0);


}



//==============================
// Intensidad según distancia
//==============================

int calcularIntensidad(long distancia){


  if(distancia <= 0){

    return 0;

  }


  if(distancia >= 80){

    return 30;

  }


  if(distancia >= 60){

    return 80;

  }


  if(distancia >= 35){

    return 150;

  }


  return 255;

}



//==============================
// Aplicar aumento por suelo
//==============================

int aplicarSuelo(int intensidad){


  if(distanciaSuelo > 0 && distanciaSuelo < 30){


    intensidad = intensidad + (intensidad * 50 / 100);


  }
  if (intensidad==0){
    intensidad=50;
  }


  if(intensidad > 255){

    intensidad = 255;

  }


  return intensidad;

}



//==============================
// Actualizar alarma
//==============================

void actualizarAlarma(){



  // Primero apagar

  apagarMotores();

  //==================================
  // ALERTA RADAR
  //==================================


  if(distanciaRadar > 5 && distanciaRadar < 80){
    

    int intensidad1 = calcularIntensidad(distanciaRadar);


    int intensidad = aplicarSuelo(intensidad1);



    if(anguloRadar <= 45){


      analogWrite(MOTOR_IZQUIERDO,intensidad);
      if (intensidad>intensidad1){
        analogWrite(MOTOR_CENTRO,50);
        analogWrite(MOTOR_FRONTAL,50);
        analogWrite(MOTOR_DERECHO,50); } 

    }


    else if(anguloRadar <= 75){


      analogWrite(MOTOR_IZQUIERDO,intensidad);

      analogWrite(MOTOR_CENTRO,intensidad);
      if (intensidad>intensidad1){
        analogWrite(MOTOR_FRONTAL,50);
        analogWrite(MOTOR_DERECHO,50); } 

    }


    else if(anguloRadar <= 105){


      analogWrite(MOTOR_CENTRO,intensidad);
      if (intensidad>intensidad1){
        analogWrite(MOTOR_IZQUIERDO,50);
        analogWrite(MOTOR_FRONTAL,50);
        analogWrite(MOTOR_DERECHO,50); } 

    }


    else if(anguloRadar <= 135){


      analogWrite(MOTOR_CENTRO,intensidad);

      analogWrite(MOTOR_DERECHO,intensidad);

      if (intensidad>intensidad1){
        analogWrite(MOTOR_IZQUIERDO,50);
        analogWrite(MOTOR_FRONTAL,50);
 } 

    }
    


    else{


      analogWrite(MOTOR_DERECHO,intensidad);

      if (intensidad>intensidad1){
        analogWrite(MOTOR_CENTRO,50);
        analogWrite(MOTOR_FRONTAL,50);
        analogWrite(MOTOR_IZQUIERDO,50); } 

    }


  }





  //==================================
  // ALERTA FRONTAL
  //==================================


  if(distanciaFrontal > 0 && distanciaFrontal < 100){



    int intensidadFrontal = calcularIntensidad(distanciaFrontal);



    intensidadFrontal = aplicarSuelo(intensidadFrontal);



    analogWrite(MOTOR_FRONTAL,intensidadFrontal);



  }



}