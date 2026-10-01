/*
====================================================
RADAR.ino
Control del servo del radar
====================================================
*/

void iniciarRadar() {

  radarServo.attach(PIN_SERVO);

  anguloRadar = ANGULO_MIN;

  radarServo.write(anguloRadar);

}


void actualizarRadar() {

  if (millis() - tRadar < PERIODO_RADAR) return;

  tRadar = millis();


  if (direccionRadar) {

    anguloRadar += PASO_SERVO;

    if (anguloRadar >= ANGULO_MAX) {

      anguloRadar = ANGULO_MAX;

      direccionRadar = false;

    }

  }

  else {

    anguloRadar -= PASO_SERVO;

    if (anguloRadar <= ANGULO_MIN) {

      anguloRadar = ANGULO_MIN;

      direccionRadar = true;

    }

  }


  radarServo.write(anguloRadar);

}