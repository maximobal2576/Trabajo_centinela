/*
====================================================
DECISION.ino
Lógica principal del bastón
====================================================
*/


//=============================================
// Convierte distancia a intensidad PWM
//=============================================

byte calcularPWM(long distancia){

  if(distancia >= 120) return 0;

  if(distancia <= 15) return PWM_MAXIMO;

  return map(distancia,120,15,PWM_MINIMO,PWM_MAXIMO);

}



//=============================================
// Actualizar decisiones
//=============================================

void tomarDecisiones(){

  // Reiniciar motores

  pwmIzquierdo = 0;
  pwmCentro = 0;
  pwmDerecho = 0;
  pwmFrontal = 0;



  //=========================================
  // POZO (Máxima prioridad)
  //=========================================

  if(distanciaSuelo <= DISTANCIA_POZO){

      pwmIzquierdo = max(pwmIzquierdo,PWM_POZO);

      pwmCentro = max(pwmCentro,PWM_POZO);

      pwmDerecho = max(pwmDerecho,PWM_POZO);

      pwmFrontal = max(pwmFrontal,PWM_POZO);

  }



  //=========================================
  // OBSTACULO FRONTAL
  //=========================================

  if(distanciaFrontal <= DISTANCIA_FRONTAL){

      pwmFrontal = max(pwmFrontal,
                       calcularPWM(distanciaFrontal));

  }



  //=========================================
  // RADAR
  //=========================================

  if(distanciaRadar <= DISTANCIA_RADAR){

      byte intensidad = calcularPWM(distanciaRadar);


      if(anguloRadar <= 45){

          pwmIzquierdo =
          max(pwmIzquierdo,intensidad);

      }

      else if(anguloRadar <= 75){

          pwmIzquierdo =
          max(pwmIzquierdo,intensidad);

          pwmCentro =
          max(pwmCentro,intensidad);

      }

      else if(anguloRadar <= 105){

          pwmCentro =
          max(pwmCentro,intensidad);

      }

      else if(anguloRadar <= 135){

          pwmCentro =
          max(pwmCentro,intensidad);

          pwmDerecho =
          max(pwmDerecho,intensidad);

      }

      else{

          pwmDerecho =
          max(pwmDerecho,intensidad);

      }

  }

}