/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Salida.cpp
 * Descripcion:
 *   Control PWM de los 3 motores vibradores hápticos via LEDC del ESP32.
 *   Conectados a través de drivers MOSFET canal N a nivel lógico 3.3V.
 **************************************************************************/
#include "Salida.h"
#include "Estado.h"
#include "Configuracion.h"

void Salida::iniciar()
{
    // Configurar 3 canales LEDC para los 3 motores vibradores
    ledcSetup(LEDC_CH_IZQ, LEDC_FREQ_HZ, LEDC_BITS);
    ledcSetup(LEDC_CH_CTR, LEDC_FREQ_HZ, LEDC_BITS);
    ledcSetup(LEDC_CH_DER, LEDC_FREQ_HZ, LEDC_BITS);

    ledcAttachPin(MOTOR_IZQUIERDO, LEDC_CH_IZQ);
    ledcAttachPin(MOTOR_CENTRO,    LEDC_CH_CTR);
    ledcAttachPin(MOTOR_DERECHO,   LEDC_CH_DER);

    // Apagar motores al inicio
    ledcWrite(LEDC_CH_IZQ, 0);
    ledcWrite(LEDC_CH_CTR, 0);
    ledcWrite(LEDC_CH_DER, 0);

    Serial.println("Drivers MOSFET para 3 motores vibradores iniciados (LEDC PWM)");
}

void Salida::actualizar()
{
    // Si hay evento de emergencia / SOS o caida, generar patron de vibracion haptica triple
    if (Estado::sistema.eventoActual == EVENTO_EMERGENCIA || Estado::sistema.eventoActual == EVENTO_CAIDA)
    {
        static unsigned long tAlerta = 0;
        static bool pulso = false;
        if (millis() - tAlerta > 200)
        {
            tAlerta = millis();
            pulso = !pulso;
            int v = pulso ? PWM_MAXIMO : 0;
            escribirMotor(LEDC_CH_IZQ, v);
            escribirMotor(LEDC_CH_CTR, v);
            escribirMotor(LEDC_CH_DER, v);
        }
        return;
    }

    escribirMotor(LEDC_CH_IZQ, Estado::salida.motorIzquierdo);
    escribirMotor(LEDC_CH_CTR, Estado::salida.motorCentro);
    escribirMotor(LEDC_CH_DER, Estado::salida.motorDerecho);
}

void Salida::escribirMotor(int canal, int valor)
{
    valor = constrain(valor, 0, 255);
    ledcWrite(canal, valor);
}
