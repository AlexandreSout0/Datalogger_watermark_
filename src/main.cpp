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
#include <ESP32Time.h>
#include <LiquidCrystal_I2C.h>


//==================================== Mapeamento de Hardware =================================== //

#define pwr_en 2 // Power Enable
#define swp_freq 23 // Frequency input
#define s0 4 // Control Multiplex
#define s1 5 // Control Multiplex

// Data e hora de inicialização do esp32
#define DIA 20
#define MES 1
#define ANO 2022
#define HORA 8
#define MINUTOS 39
#define SEGUNDOS 30

//=============================================================================================== //




//============================================= Funções ========================================= //

int ReadFrequency (int swp);
void getDataDebug();

ESP32Time rtc;
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


//=============================================================================================== //



void setup()
{
  Serial.begin(9600);
  Serial.println("Start");
  rtc.setTime(SEGUNDOS, MINUTOS, HORA, DIA, MES, ANO);  // 17th Jan 2021 15:24:30
  pinMode(pwr_en, OUTPUT);
  pinMode (s0, OUTPUT);
  pinMode (s1, OUTPUT);
  pinMode(swp_freq, INPUT);
  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(pwr_en, LOW);
  lcd.init(); // initialize the lcd 
  lcd.backlight(); // display light


  
}

void loop()
{

  getDataDebug();
  //delay(100);
  

  int irrometer1 = 0;
  int irrometer2 = 0;
  int irrometer3 = 0;

  /****** SWP_1 *******/
  digitalWrite(pwr_en, HIGH);//switch ON sensor 
  delay(100);
  irrometer1 = ReadFrequency(1);
  /****** SWP_2 *******/
  delay(100);
  irrometer2 = ReadFrequency(2);
  /****** SWP_3 *******/
  delay(100);
  irrometer3 = ReadFrequency(3);
  delay(100);
  digitalWrite(pwr_en, LOW);//switch off sensor

  lcd.clear();
  lcd.setCursor(8,0);
  lcd.print(rtc.getTime("%d/%m/%y"));

  lcd.setCursor(0,1);
  lcd.printf("%d",irrometer1);
 
  lcd.setCursor(4,1);
  lcd.printf("%d",irrometer2);

  lcd.setCursor(8,1);
  lcd.printf("%d",irrometer3);

  lcd.setCursor(11,1);
  lcd.print("[kpa]");




}

int ReadFrequency (int swp)
{

  delay(100);
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

  

  delay(500);
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

  if (irrometerfrequencyTemp > 485 && irrometerfrequencyTemp <= 600)
  {
    soil_moisture  = 100 - (irrometerfrequencyTemp - 485) * 0.2174;
  }
  
  if (irrometerfrequencyTemp > 600 && irrometerfrequencyTemp <= 770)
  {
    soil_moisture = 75 - (irrometerfrequencyTemp - 600) * 0.1176;
  }

  if (irrometerfrequencyTemp > 770 && irrometerfrequencyTemp <= 1110)
  {
    soil_moisture = 55 - (irrometerfrequencyTemp - 770) * 0.05884;
  }

  if (irrometerfrequencyTemp > 1110 && irrometerfrequencyTemp <= 2820)
  {
   soil_moisture = 35 - (irrometerfrequencyTemp - 1110) * 0.01170;
  }
  
   if (irrometerfrequencyTemp > 2820 && irrometerfrequencyTemp <= 4330)
  {
    soil_moisture = 15 - (irrometerfrequencyTemp - 2820) * 0.003974;
  }

  if (irrometerfrequencyTemp > 4330 && irrometerfrequencyTemp <= 6430)
  {
    soil_moisture = 9 - (irrometerfrequencyTemp - 4600) * 0.004286;
  }

  if (irrometerfrequencyTemp > 6430)
  {
    soil_moisture = 0;
  }
  
  return soil_moisture;
}


void getDataDebug()
{
  digitalWrite(pwr_en, HIGH);//switch ON sensor 
  delay(100);

  Serial.println("====================================================================");
  String time = rtc.getTime("%d/%m/%y %H:%M:%S");
  /****** SWP_1 *******/
  int irrometerfrequencyTemp1 = ReadFrequency(1);
  lcd.setCursor(0,1);
  Serial.println(String(time) + " Primary Soil Sensor = " + String(irrometerfrequencyTemp1) + " Kpa");
  delay(100);
  time = rtc.getTime("%d/%m/%y %H:%M:%S");
   /****** SWP_2 *******/
  int irrometerfrequencyTemp2 = ReadFrequency(2);
  lcd.setCursor(4,1);
  Serial.println(String(time) + " Secondary Soil Sensor = " + String(irrometerfrequencyTemp2) + " Kpa");
  delay(100);
  time = rtc.getTime("%d/%m/%y %H:%M:%S");
  /****** SWP_3 *******/
  int irrometerfrequencyTemp3 = ReadFrequency(3);
  lcd.setCursor(4,1);
  Serial.println(String(time) + " Third Soil Sensor = " + String(irrometerfrequencyTemp3) + " Kpa");
  delay(100);
  time = rtc.getTime("%d/%m/%y %H:%M:%S");
  /****** SWP_4 *******/
  int irrometerfrequencyTemp4 = ReadFrequency(4);
  Serial.println(String(time) + " Fourth Soil Sensor = " + String(irrometerfrequencyTemp4) + " Kpa");
  delay(100);
  Serial.println("====================================================================");

  digitalWrite(pwr_en, LOW);//switch off sensor
}
