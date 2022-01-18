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



//==================================== Mapeamento de Hardware =================================== //

#define pwr_en 2 // Power Enable
#define swp_freq 23 // Frequency input
#define s0 4 // Control Multiplex
#define s1 5 // Control Multiplex


//=============================================================================================== //




//============================================= Funções ========================================= //

float ReadFrequency (int swp);
void getData();



//=============================================================================================== //




void setup()
{
  Serial.begin(9600);
  Serial.println("Start");

  pinMode(pwr_en, OUTPUT);
  pinMode (s0, OUTPUT);
  pinMode (s1, OUTPUT);
  pinMode(swp_freq, INPUT);
  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(pwr_en, LOW);
  
}

void loop()
{
  getData();
  delay(5000);
}

float ReadFrequency (int swp)
{

  delay(200);
  if (swp == 1)
  {
    digitalWrite(s0,LOW);
    digitalWrite(s1,LOW);
  }
  if (swp == 2)
  {
    digitalWrite(s0,LOW);
    digitalWrite(s1,HIGH);
  }
  if (swp == 3)
  {
    digitalWrite(s0,HIGH);
    digitalWrite(s1,LOW);
  }
  if (swp == 4)
  {
    digitalWrite(s0,HIGH);
    digitalWrite(s1,HIGH);
  }

  delay(1000);
  float totalTime = 0;
  int highPulseTime = 0;
  int lowPulseTime = 0;
  int sample = 10;
  float irrometerfrequencyTemp = 0;
  float frequencyTemp = 0;
  float freqcumulative = 0;

  for (int i = 0 ; i < sample; i++)
  {
      highPulseTime = 0;
      lowPulseTime = 0;
      totalTime = 0;
/*     
                    high
        |--|  |--|  |--|  |--|  |--|
      __|  |__|  |__|  |__|  |__|  |____  signal PWM of integrad circuit 555.
            low
-----------------------------------------------> time

*/
      highPulseTime = pulseIn(swp_freq, HIGH); // Time the signal is high
      lowPulseTime = pulseIn(swp_freq, LOW); // Time the signal is low
      totalTime = highPulseTime + lowPulseTime; // Sum time high and low
      if (totalTime <= 0)
      {
        totalTime = 1000000;
      }
      frequencyTemp = 1000000 / totalTime;
      freqcumulative = freqcumulative + frequencyTemp;
      delay(10);

  }
  irrometerfrequencyTemp = freqcumulative / sample;
/*    
      kPa = 0                             for Hz > 6430
      kPa = 9 - (Hz - 4600) * 0.004286    for 4330 <= Hz <= 6430
      kPa = 15 - (Hz - 2820) * 0.003974   for 2820 <= Hz <= 4330
      kPa = 35 - (Hz - 1110) * 0.01170    for 1110 <= Hz <= 2820
      kPa = 55 - (Hz - 770) * 0.05884     for 770 <= Hz <= 1110
      kPa = 75 - (Hz - 600) * 0.1176      for 600 <= Hz <= 770
      kPa = 100 - (Hz - 485) * 0.2174     for 485 <= Hz <= 600
      kPa = 200 - (Hz - 293) * 0.5208     for 293 <= Hz <= 485
      kPa = 200                           for Hz < 293
*/

  int soil_moisture;

  if (irrometerfrequencyTemp < 293)
  {
    soil_moisture = 200;
  }

  if (irrometerfrequencyTemp >= 293 && irrometerfrequencyTemp <= 485)
  {
    soil_moisture = 200 - (irrometerfrequencyTemp - 293) * 0.5208;

  }

  if (irrometerfrequencyTemp > 293 && irrometerfrequencyTemp < 485)
  {
    soil_moisture  = 100 - (irrometerfrequencyTemp - 485) * 0.2174;
  }
  
  if (irrometerfrequencyTemp < 293)
  {

  }
  if (irrometerfrequencyTemp < 293)
  {

  }
  if (irrometerfrequencyTemp < 293)
  {

  }



}


void getData()
{
  digitalWrite(pwr_en, HIGH);//switch ON sensor
  delay(100);

  /****** SWP_1 *******/
  float irrometerfrequencyTemp1 = ReadFrequency(1);
  Serial.println("Primary Soil Sensor = " + String(irrometerfrequencyTemp1) + " Hz");

   /****** SWP_2 *******/
  float irrometerfrequencyTemp2 = ReadFrequency(2);
  Serial.println("Secondary Soil Sensor = " + String(irrometerfrequencyTemp2) + " Hz");

  /****** SWP_3 *******/
  float irrometerfrequencyTemp3 = ReadFrequency(3);
  Serial.println("Third Soil Sensor = " + String(irrometerfrequencyTemp3) + " Hz");

  /****** SWP_4 *******/
  float irrometerfrequencyTemp4 = ReadFrequency(4);
  Serial.println("Fourth Soil Sensor = " + String(irrometerfrequencyTemp4) + " Hz");

  digitalWrite(pwr_en, LOW);//switch off sensor
}
