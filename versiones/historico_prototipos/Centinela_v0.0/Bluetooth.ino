/*
  Bluetooth.ino
  Comunicación Bluetooth
*/


//==============================
// Inicializar Bluetooth
//==============================
#include "Variables.h"
void iniciarBluetooth(){


  Serial1.begin(9600);


  Serial.println("Bluetooth iniciado");


}



//==============================
// Actualizar Bluetooth
//==============================

void actualizarBluetooth(){



  static unsigned long ultimoEnvio = 0;



  // Enviar datos cada 1 segundo

  if(millis() - ultimoEnvio >= 1000){


    ultimoEnvio = millis();



    Serial1.print("Radar:");

    Serial1.print(distanciaRadar);


    Serial1.print("cm | Frontal:");

    Serial1.print(distanciaFrontal);



    Serial1.print("cm | Suelo:");

    Serial1.print(distanciaSuelo);



    Serial1.print("cm | Angulo:");

    Serial1.print(anguloRadar);



    Serial1.print(" | Caida:");



    if(caidaDetectada){

      Serial1.println("SI");

    }

    else{

      Serial1.println("NO");

    }



  }



}