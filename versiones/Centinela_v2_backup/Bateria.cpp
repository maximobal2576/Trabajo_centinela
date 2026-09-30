/**************************************************************************
 * Proyecto : Centinela v2.0
 * Archivo  : Bateria.cpp
 * Descripcion:
 *   Medicion de bateria via ADC de 12 bits del ESP32 (GPIO 34).
 *   v2: la resolucion es 12 bits (0-4095) en lugar de 10 bits del AVR.
 *   Ajustar ADC_LECTURA_VACIA y ADC_LECTURA_LLENA en Configuracion.h
 *   segun el divisor de tension real del hardware.
 **************************************************************************/
#include "Bateria.h"
#include "Estado.h"
#include "Configuracion.h"

void Bateria::iniciar()
{
    // GPIO 34 es solo entrada (no necesita pinMode OUTPUT)
    pinMode(PIN_BATERIA, INPUT);
    // Resolucion ADC del ESP32: 12 bits por defecto
    analogReadResolution(12);
}

void Bateria::actualizar()
{
    static int lecturaSuavizada = 0;

    int lectura = analogRead(PIN_BATERIA);

    if (lecturaSuavizada == 0)
    {
        lecturaSuavizada = lectura;
    }
    else
    {
        // Filtro exponencial suave (7/8 viejo + 1/8 nuevo)
        lecturaSuavizada = (lecturaSuavizada * 7 + lectura) / 8;
    }

    Estado::sistema.bateria = calcularPorcentaje(lecturaSuavizada);

    if (Estado::sistema.bateria < BATERIA_UMBRAL_BAJA)
    {
        Estado::sistema.eventoActual    = EVENTO_BATERIA_BAJA;
        Estado::sistema.prioridadActual = PRIORIDAD_ALTA;
        Estado::sistema.direccion       = DIRECCION_NINGUNA;
    }
    else if (Estado::sistema.bateria >= BATERIA_UMBRAL_RECUPERACION)
    {
        if (Estado::sistema.eventoActual == EVENTO_BATERIA_BAJA)
        {
            Estado::sistema.eventoActual    = EVENTO_NORMAL;
            Estado::sistema.prioridadActual = PRIORIDAD_BAJA;
        }
    }
}

int Bateria::calcularPorcentaje(int lectura)
{
    // Mapeo lineal entre lectura vacia y llena (calibrar con hardware real)
    int porcentaje = map(lectura, ADC_LECTURA_VACIA, ADC_LECTURA_LLENA, 0, 100);
    return constrain(porcentaje, 0, 100);
}
