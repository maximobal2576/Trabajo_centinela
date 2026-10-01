/**************************************************************************
 * Proyecto : Centinela v1.0
 * Archivo  : MPU6050.cpp
 * Descripción:
 * Detector inteligente de caída.
 **************************************************************************/

#include "MPU6050.h"



namespace MPU6050_Module
{


                // Set the state to MPU_CAIDA and mark the fall detected
                tiempoCaidaInicio = millis();
                Estado::sensores.caidaDetectada = true;
//------------------------------------------------------
// Objeto MPU
//------------------------------------------------------

MPU6050 mpu(Wire);



//------------------------------------------------------
// Variables internas
//------------------------------------------------------

EstadoDeteccion estado = MPU_NORMAL;



float offsetAx = 0;
float offsetAy = 0;
float offsetAz = 0;



float aceleracionFiltro = 1.0;

float giroFiltro = 0;



float pitchGolpe = 0;

float rollGolpe = 0;



unsigned long tiempoGolpe = 0;

unsigned long tiempoConfirmacion = 0;

unsigned long tiempoCaidaInicio = 0;




//------------------------------------------------------
// Parámetros
//------------------------------------------------------

const float UMBRAL_IMPACTO = 2.3;

const float UMBRAL_ANGULO = 45.0;

const float UMBRAL_GIRO = 160.0;



const unsigned long ESPERA_GOLPE = 200;

const unsigned long TIEMPO_MAX_CONFIRMACION = 2500;
const unsigned long TIEMPO_RECUPERACION_CAIDA = 5000;




//------------------------------------------------------
// Inicialización
//------------------------------------------------------

void iniciar()
{

    Wire.begin();
            // Intentar detectar recuperación: si hay movimiento fuerte
            // o han pasado unos segundos desde la caída y las inclinaciones
            // vuelven a valores razonables, reiniciamos el detector.

            if(tiempoCaidaInicio == 0)
            {
                tiempoCaidaInicio = millis();
            }

            bool movimientoRecuperacion = (giroFiltro > UMBRAL_GIRO);
            bool angulosNormales = (abs(pitch) < (UMBRAL_ANGULO / 2) && abs(roll) < (UMBRAL_ANGULO / 2));

            if(movimientoRecuperacion || (millis() - tiempoCaidaInicio > TIEMPO_RECUPERACION_CAIDA && angulosNormales))
            {
                estado = MPU_NORMAL;
                Estado::sensores.caidaDetectada = false;
                tiempoCaidaInicio = 0;
            }


    mpu.begin();


    delay(100);



    // Calibración inicial

    float sumaX = 0;

    float sumaY = 0;

    float sumaZ = 0;



    for(int i = 0; i < 100; i++)
    {

        mpu.update();



        sumaX += mpu.getAccX();

        sumaY += mpu.getAccY();

        sumaZ += mpu.getAccZ();



        delay(5);

    }



    offsetAx = sumaX / 100.0;

    offsetAy = sumaY / 100.0;

    offsetAz = (sumaZ / 100.0) - 1.0;



    Estado::sensores.caidaDetectada = false;

    Estado::sensores.movimientoDetectado = false;



}





//------------------------------------------------------
// Actualización principal
//------------------------------------------------------

void actualizar()
{


    mpu.update();



    float ax =
    mpu.getAccX() - offsetAx;


    float ay =
    mpu.getAccY() - offsetAy;


    float az =
    mpu.getAccZ() - offsetAz;



    float gx = mpu.getGyroX();

    float gy = mpu.getGyroY();

    float gz = mpu.getGyroZ();





    //--------------------------------------------------
    // Aceleración total
    //--------------------------------------------------

    float aceleracion =
    sqrt(
        ax*ax +
        ay*ay +
        az*az
    );



    aceleracionFiltro =
    (aceleracionFiltro * 0.85) +
    (aceleracion * 0.15);





    //--------------------------------------------------
    // Giro
    //--------------------------------------------------

    float giro =
    sqrt(
        gx*gx +
        gy*gy +
        gz*gz
    );



    giroFiltro =
    (giroFiltro * 0.8) +
    (giro * 0.2);





    //--------------------------------------------------
    // Ángulos
    //--------------------------------------------------

    float pitch =
    mpu.getAngleX();


    float roll =
    mpu.getAngleY();




    Estado::sensores.inclinacionX = pitch;

    Estado::sensores.inclinacionY = roll;





    Estado::sensores.movimientoDetectado =
    (giroFiltro > UMBRAL_GIRO);





    //--------------------------------------------------
    // Máquina de estados
    //--------------------------------------------------

    switch(estado)
    {


        case MPU_NORMAL:


            if(aceleracionFiltro > UMBRAL_IMPACTO)
            {


                estado = MPU_GOLPE;



                tiempoGolpe = millis();



                pitchGolpe = pitch;

                rollGolpe = roll;



            }


        break;






        case MPU_GOLPE:


            if(millis() - tiempoGolpe > ESPERA_GOLPE)
            {

                estado = MPU_CONFIRMACION;


                tiempoConfirmacion = millis();


            }


        break;







        case MPU_CONFIRMACION:
        {


            float cambioPitch =
            abs(pitch - pitchGolpe);



            float cambioRoll =
            abs(roll - rollGolpe);



            bool cambioAngulo =
            (
                cambioPitch > UMBRAL_ANGULO ||
                cambioRoll > UMBRAL_ANGULO
            );



            bool giroFuerte =
            (
                giroFiltro > UMBRAL_GIRO
            );





            if(cambioAngulo && giroFuerte)
            {

                estado = MPU_CAIDA;


                Estado::sensores.caidaDetectada = true;


            }






            if(millis() - tiempoConfirmacion >
               TIEMPO_MAX_CONFIRMACION)
            {

                estado = MPU_NORMAL;


            }


        }

        break;







        case MPU_CAIDA:


            Estado::sensores.caidaDetectada = true;



        break;


    }



}







//------------------------------------------------------
// Reinicio
//------------------------------------------------------

void reiniciar()
{

    estado = MPU_NORMAL;


    Estado::sensores.caidaDetectada = false;


}




EstadoDeteccion obtenerEstado()
{

    return estado;

}



}