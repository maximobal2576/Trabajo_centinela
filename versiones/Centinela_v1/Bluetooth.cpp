/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : Bluetooth.cpp
 * Descripción:
 * Comunicación serial con módulo HC-05.
 **************************************************************************/


#include "Bluetooth.h"

#include "Estado.h"



#define BLUETOOTH_SERIAL Serial1



void Bluetooth::iniciar()
{

    BLUETOOTH_SERIAL.begin(
        BLUETOOTH_BAUDRATE
    );


    Estado::sistema.bluetoothConectado = false;

}



void Bluetooth::actualizar()
{

    recibirComandos();


    enviarEstado();


}



// =====================================================
// ENVIAR INFORMACION A LA APP
// =====================================================

void Bluetooth::enviarEstado()
{

    BLUETOOTH_SERIAL.print("EVENTO:");
    BLUETOOTH_SERIAL.println(Estado::sistema.eventoActual);

    BLUETOOTH_SERIAL.print("DIRECCION:");
    BLUETOOTH_SERIAL.println(Estado::sistema.direccion);

    BLUETOOTH_SERIAL.print("BATERIA:");
    BLUETOOTH_SERIAL.println(Estado::sistema.bateria);

    BLUETOOTH_SERIAL.print("AUDIO_OCUPADO:");
    BLUETOOTH_SERIAL.println(Estado::sistema.audioOcupado ? 1 : 0);

    BLUETOOTH_SERIAL.print("BLUETOOTH:");
    BLUETOOTH_SERIAL.println(Estado::sistema.bluetoothConectado ? 1 : 0);

    BLUETOOTH_SERIAL.print("ESTADO:");
    BLUETOOTH_SERIAL.println(Estado::sistema.estado);

}



// =====================================================
// RECIBIR COMANDOS
// =====================================================

void Bluetooth::recibirComandos()
{

    if(BLUETOOTH_SERIAL.available())
    {

        Estado::sistema.bluetoothConectado = true;


        char comando =
        BLUETOOTH_SERIAL.read();



        switch(comando)
        {

            case 'D':

                Estado::sistema.estado =
                SISTEMA_DIAGNOSTICO;

            break;

            case 'N':

                Estado::sistema.estado =
                SISTEMA_NORMAL;

            break;

        }

    }
    else if(BLUETOOTH_SERIAL.availableForWrite() > 0)
    {

        Estado::sistema.bluetoothConectado = true;

    }
    else
    {

        Estado::sistema.bluetoothConectado = false;

    }

}