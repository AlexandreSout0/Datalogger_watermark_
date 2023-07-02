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

#include "FS.h"
#include <SPIFFS.h>


//==================================== Mapeamento de Hardware =================================== //

#define pwr_en 2 // Power Enable
#define swp_freq 23 // Frequency input
#define s0 4 // Control Multiplex
#define s1 5 // Control Multiplex

#define pressure_call 26 // monitor de pressão irrigação
#define pressure_back 25 // monitor de pressão irrigação
#define DEBOUNCE1 60000
#define DEBOUNCE2 20000

// Data e hora de inicialização do esp32
#define DIA 28
#define MES 10
#define ANO 2022
#define HORA 9
#define MINUTOS 28
#define SEGUNDOS 39


#define TIME_ON 10000 // Time before to sending logs on serial port
#define TIME_LOG 7200000000 // INTERRUPÇÃO: 3600000000  = 1 hora (60000000 1 minuto)   900000000

//=============================================================================================== //




//============================================= Funções ========================================= //
bool writeFile(String values, String pathFile, bool appending); // Write file
String readFile(String pathFile);// Read file
bool deleteFile(String pathFile); // Delete file
void renameFile(String pathFileFrom, String pathFileTo);// Rename file
bool formatFS() ; // Formart file system
void listFiles(String path); // list files from directoy

int ReadFrequency (int swp);
void DataLogger();
bool flag_i1 = NULL;

double timeon;
double timeoff;
double total;
double globalTime;
String timeOn;
String timeOff;


ESP32Time rtc;
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


//=============================================================================================== //

volatile int interruptCounter;
int totalInterruptCounter;
 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
 
}


void setup()
{
  Serial.begin(9600);
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

  Serial.println("\nDescontingenciamento");
  delay(TIME_ON);
  lcd.setCursor(0,0);
  lcd.print("DESCONTING...");

  Serial.println("\n -- Start Log tensiometry -- ");
  SPIFFS.begin(true);
  File rFile = SPIFFS.open("/Log_.txt", "r");
  String values;
  while (rFile.available()) {
        values = rFile.readString();
        Serial.println(values);
        values = "";
  }
  rFile.close();
  Serial.println("\n -- Final Log tensiometry -- ");

  Serial.println("\n -- Start Log Irrigation -- ");
  SPIFFS.begin(true);
  File rFile2 = SPIFFS.open("/Log_IRR.txt", "r");
  String values2;
  while (rFile2.available()) {
        values2 = rFile2.readString();
        Serial.println(values2);
        values2 = "";
  }
  rFile2.close();
  Serial.println("\n -- Final Log Irrigation -- ");

  delay(TIME_ON);

  formatFS();

  delay(1000);
  Serial.println("\n -- Final Log -- ");
  Serial.println("\n");
  Serial.println("\n");

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, TIME_LOG , true);
  timerAlarmEnable(timer);
  
}



void loop()
{
 
  pinMode (pressure_call, OUTPUT); // Define o pino como saida
  pinMode (pressure_back, INPUT); // define o pino como entrada
  digitalWrite(pressure_call, HIGH); // nivel logico alto para pino que vai para o sensor de pressão


  if (analogRead(pressure_back) < 2000 && flag_i1 != true ){
    delay(DEBOUNCE1);
    if (analogRead(pressure_back) < 2000 && flag_i1 != true ){
      
      timeOn = rtc.getTime("%d/%m/%y %H:%M:%S");
      Serial.println(" Sistema com Pressão ");
      Serial.print(timeOn);
      timeon = millis();
      flag_i1 = true;
    }
    else{
      return;
    }
  }

  if (analogRead(pressure_back) > 3000 && flag_i1 == true ) {
    delay(DEBOUNCE2);
    if (analogRead(pressure_back) > 3000 && flag_i1 == true ) {
      timeOff = rtc.getTime(" %d/%m/%y %H:%M:%S ");
      Serial.println(" Sistema sem Pressão ");
      Serial.println(timeOff);
      timeoff = millis();
      total = timeoff-timeon;
      total = total/60000;
      Serial.println(total);
      writeFile("---> Start irrigation: " + timeOn + " End irrigation: " + timeOff + "Total: " + total , "/Log_IRR.txt" , true);
      flag_i1 = false;
    }
    else{
      return;
    }
  }
  //int teste23 = analogRead(pressure_back);
  //Serial.println(teste23);

  int irrometer1 = 0;
  int irrometer2 = 0;
  int irrometer3 = 0;

  
  digitalWrite(pwr_en, HIGH);//switch ON sensor 

  /****** SWP_1 *******/
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

  lcd.setCursor(0,0);
  lcd.print("Tensi ");
  
  if (flag_i1 == 1){
      lcd.setCursor(6,0);
      lcd.print("*");
  }
  else{
      lcd.setCursor(6,0);
      lcd.print("-");
  }

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




  if (interruptCounter > 0) {
 
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
 
    totalInterruptCounter++;
    DataLogger();
 
  }

}



//==============================================================================//
//==============================================================================//
//==============================================================================//
//==============================================================================//







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


/*--- Write File ---*/
bool writeFile(String values, String pathFile, bool appending) {
  char *mode = "w"; //open for writing (creates file if it doesn't exist). Deletes content and overwrites the file.
  if (appending) mode = "a"; //open for appending (creates file if it doesn't exist)
  //Serial.println("- Writing file: " + pathFile);
  //Serial.println("- Values: " + values);
  SPIFFS.begin(true);
  File wFile = SPIFFS.open(pathFile, mode);
  if (!wFile) {
    Serial.println("- Failed to write file.");
    return false;
  } else {
    wFile.println(values);
   // Serial.println("- Written!");
  }
  wFile.close();
  return true;
}

/*--- Read file ---*/
String readFile(String pathFile) {
  Serial.println("- Reading file: " + pathFile);
  SPIFFS.begin(true);
  File rFile = SPIFFS.open(pathFile, "r");
  String values;
  if (!rFile) {
    Serial.println("- Failed to open file.");
  } else {
    while (rFile.available()) {
      values += rFile.readString();
    }
    Serial.println("- File values: " + values);
  }
  rFile.close();
  return values;
}

/*--- Delete File ---*/
bool deleteFile(String pathFile) {
  Serial.println("- Deleting file: " + pathFile);
  SPIFFS.begin(true);
  if (!SPIFFS.remove(pathFile)) {
    Serial.println("- Delete failed.");
    return false;
  } else {
    Serial.println("- File deleted!");
    return true;
  }
}

/*--- Rename file---*/
void renameFile(String pathFileFrom, String pathFileTo) {
  Serial.println("- Renaming file " + pathFileFrom + " to " + pathFileTo);
  SPIFFS.begin(true);
  if (!SPIFFS.rename(pathFileFrom, pathFileTo)) {
    Serial.println("- Rename failed.");
  } else {
    Serial.println("- File renamed!");
  }
}

/*--- Format File System ---*/
bool formatFS() {

  Serial.println("- Formatting file system...");
  //lcd.clear();
  //lcd.setCursor(0,1);
  //lcd.print("Formating system");
  SPIFFS.begin(true);
  if (!SPIFFS.format()) {
    Serial.println("- Format failed.");
    return false;
  } else {
    Serial.println("- Formatted!");
    return true;
  }
  //lcd.clear();
}

/*--- List Files From Directory ---*/
void listFiles(String path) {
  Serial.println("- Listing files: " + path);
  SPIFFS.begin(true);
  File root = SPIFFS.open(path);
  if (!root) {
    Serial.println("- Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("- Not a directory: " + path);
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("- Dir: ");
      Serial.println(file.name());
    } else {
      Serial.print("- File: ");
      Serial.print(file.name());
      Serial.print("\tSize: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

/*--- Print Serial Port Debug---*/
void DataLogger()
{
  digitalWrite(pwr_en, HIGH);//switch ON sensor 
  delay(100);

  String time = rtc.getTime("%d/%m/%y %H:%M:%S");
  
  /****** SWP_1 *******/
  int irrometerfrequencyTemp1 = ReadFrequency(1);
  lcd.setCursor(0,1);
  Serial.println(String(time) + " Primary Soil Sensor = " + String(irrometerfrequencyTemp1) + " Kpa");
  delay(100);

   /****** SWP_2 *******/
  int irrometerfrequencyTemp2 = ReadFrequency(2);
  lcd.setCursor(4,1);
  Serial.println(String(time) + " Secondary Soil Sensor = " + String(irrometerfrequencyTemp2) + " Kpa");
  delay(100);

  /****** SWP_3 *******/
  int irrometerfrequencyTemp3 = ReadFrequency(3);
  lcd.setCursor(4,1);
  Serial.println(String(time) + " Third Soil Sensor = " + String(irrometerfrequencyTemp3) + " Kpa");
  delay(100);


  /****** SWP_4 *******/
  //int irrometerfrequencyTemp4 = ReadFrequency(4);
  //Serial.println(String(time) + " Fourth Soil Sensor = " + String(irrometerfrequencyTemp4) + " Kpa");

  //delay(100);

  writeFile("---> " + time + " Sensor 30 cm " + irrometerfrequencyTemp1 + " [-kpa] " + " Sensor 60 cm " + irrometerfrequencyTemp2 + " [-kpa] " + " Sensor 90 cm " + irrometerfrequencyTemp3 + " [-kpa] ", "/Log_.txt", true);


  digitalWrite(pwr_en, LOW);//switch off sensor
}

