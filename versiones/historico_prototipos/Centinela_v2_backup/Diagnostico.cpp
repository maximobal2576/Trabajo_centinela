/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Diagnostico.cpp
 * Descripcion:
 *   Monitor serial del estado interno via USB (Serial0 del ESP32).
 *   v2: agrega info de audio stream (estado I2S) y BT SPP.
 **************************************************************************/
#include "Diagnostico.h"
#include "Estado.h"
#include "Configuracion.h"

void Diagnostico::iniciar()
{
    Serial.println("Diagnostico iniciado (ESP32 v2.0)");
}

void Diagnostico::actualizar()
{
    if (!MODO_DIAGNOSTICO) return;

    static unsigned long ultimoDiagnostico = 0;
    if (millis() - ultimoDiagnostico < TIEMPO_DIAGNOSTICO) return;

    ultimoDiagnostico = millis();
    imprimirEstado();
}

void Diagnostico::imprimirEstado()
{
    Serial.println("====================");
    Serial.print("Frontal:    "); Serial.print(Estado::sensores.frontal);        Serial.println(" cm");
    Serial.print("Suelo:      "); Serial.print(Estado::sensores.suelo);          Serial.println(" cm");
    Serial.print("Radar Izq:  "); Serial.print(Estado::sensores.radarIzquierda); Serial.println(" cm");
    Serial.print("Radar Ctr:  "); Serial.print(Estado::sensores.radarCentro);    Serial.println(" cm");
    Serial.print("Radar Der:  "); Serial.print(Estado::sensores.radarDerecha);   Serial.println(" cm");
    Serial.print("Angulo:     "); Serial.println(Estado::sensores.anguloRadar);
    Serial.print("Evento:     "); Serial.println(Estado::sistema.eventoActual);
    Serial.print("Direccion:  "); Serial.println(Estado::sistema.direccion);
    Serial.print("Bateria:    "); Serial.print(Estado::sistema.bateria);        Serial.println(" %");
    Serial.print("BT:         "); Serial.println(Estado::sistema.bluetoothConectado ? "CONECTADO" : "DESCONECTADO");
    Serial.print("Audio I2S:  "); Serial.println(Estado::sistema.audioStreamActivo ? "STREAMING" : "INACTIVO");
    Serial.print("AccX/Y/Z:   ");
    Serial.print(Estado::sensores.inclinacionX, 2); Serial.print(" / ");
    Serial.print(Estado::sensores.inclinacionY, 2); Serial.print(" / ");
    Serial.println(Estado::sensores.inclinacionZ, 2);
    Serial.println("====================");
}
