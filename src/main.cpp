/*=================================================================================================
 __    __   __   _______ .______      ______   ___   ___  __   _______    ______   
|  |  |  | |  | |       \ |   _  \   /  __  \  \  \ /  / |  | |       \  /  __  \  
|  |__|  | |  | |  .--.  ||  |_)  | |  |  |  |  \  V  /  |  | |  .--.  ||  |  |  | 
|   __   | |  | |  |  |  ||      /  |  |  |  |   >   <   |  | |  |  |  ||  |  |  | 
|  |  |  | |  | |  '--'  ||  |\  \ .|  `--'  |  /  .  \  |  | |  '--'  ||  `--'  | 
|__|  |__| |__| |_______/ | _| `._|  \______/  /__/ \__\ |__| |_______/  \______/  
                                                                    Alexandre Souto 


 -> Projeto DataLogger para Sensores de Solo WATERMARK WS200 com circuito para despolarização 
 dos sensores
 -> Monitoramento de tempo de irrigação
 -> Microcontrolado Esp32

  kPa = 0                             for Hz > 6430
  kPa = 9 - (Hz - 4600) * 0.004286    for 4330 <= Hz <= 6430
  kPa = 15 - (Hz - 2820) * 0.003974   for 2820 <= Hz <= 4330
  kPa = 35 - (Hz - 1110) * 0.01170    for 1110 <= Hz <= 2820
  kPa = 55 - (Hz - 770) * 0.05884     for 770 <= Hz <= 1110
  kPa = 75 - (Hz - 600) * 0.1176      for 600 <= Hz <= 770
  kPa = 100 - (Hz - 485) * 0.2174     for 485 <= Hz <= 600
  kPa = 200 - (Hz - 293) * 0.5208     for 293 <= Hz <= 485
  kPa = 200                           for Hz < 293
  
 by:Alexandre Souto

   ARDUINO
   Board: ESP32 DOIT DEVKIT 
   Compilador: Arduino IDE 1.8.13

   Autor: Alexandre Souto
   Data:  Novembro 2022

=================================================================================================*/

#include <Arduino.h>

void setup()
{

}

void loop()

{
  
}