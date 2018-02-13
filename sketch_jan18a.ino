#include "AntaresESPHTTP.h"

#define ACCESSKEY "0af141272a490be9:9e9c25bfafa000c1"
#define WIFISSID "Astinet lt.15"
#define PASSWORD "telkom2017"

#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
// Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

String projectName = "SmartChicken";
String deviceName1 = "TemperatureSensor";
String deviceName2 = "GasSensor";
String deviceName3 = "ExFan";

Antares antares(ACCESSKEY);

  int16_t adc0, adc1;
  
int pin = D0;
int pwm;
float tempPin = A0;
float temp = 0;
float smoke = 0;
int adcvalue;
float vin;
float sensor_volt;
float RS_gas;
float ratio;
float x;
int led1 = D6;
int led2 = D7;
int led3 = D8;

float a, b, c;
float SensorSuhu = 0, SensorAsap = 0;
float SuhuNormal, SuhuAgakTinggi, SuhuTinggi, SuhuSangatTinggi;
float AsapRendah, AsapSedang, AsapTinggi;
float MemberSuhu, MemberAsap;
float hasil[11];
float Min[11];
float Output = 0;
int LOT=50, ALOT=100, SOT=150, ACOT=200, COT=255;

void setup()
{
  Serial.begin(9600);
    Wire.begin(D2,D1);
    ads.begin();
pinMode(led1, OUTPUT);
pinMode(led2, OUTPUT);
pinMode(led3, OUTPUT);

    antares.setDebug(true);
    antares.wifiConnection(WIFISSID,PASSWORD);
  

}

void loop()
{
  
  Suhu();
  SensorSuhu = adc0; 

  Asap();
  SensorAsap = smoke;

  membershipfuzzy();
  defuzifikasi();
  Out();

  pwm = Output;
  Serial.print("\t Suhu = ");
  Serial.print(SensorSuhu);
  Serial.print("\t Asap = ");
  Serial.print(SensorAsap);
  Serial.print("\t Output = ");
  Serial.print(Output);
  Serial.print("\t PWM = ");
  Serial.print(pwm);
  Serial.print("\t RS_ratio = ");
  Serial.print(RS_gas);
  Serial.print("\t Tegangan = ");

  Serial.print(sensor_volt);
  Serial.println("");

  kirim();
    
  delay(10000);
}

void defuzifikasi()
{

  Min[0] = min(SuhuNormal, AsapRendah);
  Min[1] = min(SuhuNormal, AsapSedang);
  Min[2] = min(SuhuNormal, AsapTinggi);

  Min[3] = min(SuhuAgakTinggi, AsapRendah);
  Min[4] = min(SuhuAgakTinggi, AsapSedang);
  Min[5] = min(SuhuAgakTinggi, AsapTinggi);

  Min[6] = min(SuhuTinggi, AsapRendah);
  Min[7] = min(SuhuTinggi, AsapSedang);
  Min[8] = min(SuhuTinggi, AsapTinggi);

  Min[9] = min(SuhuSangatTinggi, AsapRendah);
  Min[10] = min(SuhuSangatTinggi, AsapSedang);
  Min[11] = min(SuhuSangatTinggi, AsapTinggi);

  hasil[0] = Min[0] * LOT;
  hasil[1] = Min[1] * ALOT;
  hasil[2] = Min[2] * SOT;
  hasil[3] = Min[3] * ALOT;
  hasil[4] = Min[4] * SOT;
  hasil[5] = Min[5] * ACOT;
  hasil[6] = Min[6] * SOT;
  hasil[7] = Min[7] * ACOT;
  hasil[8] = Min[8] * COT;
  hasil[9] = Min[9] * ACOT;
  hasil[10] = Min[10] * COT;
  hasil[11] = Min[11] * COT;

  Output = ((hasil[0]+hasil[1]+hasil[2]+hasil[3]+hasil[4]+hasil[5]+hasil[6]+hasil[7]+hasil[8]+
      hasil[9]+hasil[10]+hasil[11])/(Min[0]+Min[1]+Min[2]+Min[3]+Min[4]+Min[5]+
      Min[6]+Min[7]+Min[8]+Min[9]+Min[10]+Min[11]));

}

float Sensor_Asap(float a, float b, float c)
{
  if ((SensorAsap >= a) && (SensorAsap < b))
  {
    MemberAsap = (SensorAsap - a) / (b - a);
  }
  if ((SensorAsap >= b) && (SensorAsap < c))
  {
    MemberAsap = (c - SensorAsap) / (c - b);
  }
  if ((SensorAsap < 1.4) || (SensorAsap > 3.5))
  {
    MemberAsap = 1;
  }
  if ((SensorAsap > c) || (SensorAsap < a))
  {
    MemberAsap = 0;
  }
}

float Sensor_Suhu(float a, float b, float c)
{
  if ((SensorSuhu >= a) && (SensorSuhu < b))
  {
    MemberSuhu = (SensorSuhu - a) / (b - a);
  }
  if ((SensorSuhu >= b) && (SensorSuhu < c))
  {
    MemberSuhu = (c - SensorSuhu) / (c - b);
  }
  if ((SensorSuhu < 28) || (SensorSuhu > 60))
  {
    MemberSuhu = 1;
  }
  if ((SensorSuhu > c) || (SensorSuhu < a))
  { 
    MemberSuhu = 0;
  }
}

void membershipfuzzy()
{
  MemberSuhu = 0;
  Sensor_Suhu(a=0, b=28, c=35);
  SuhuNormal = MemberSuhu;
  Sensor_Suhu(a=28, b=35, c=40);
  SuhuAgakTinggi = MemberSuhu;
  Sensor_Suhu(a=35, b=40, c=60);
  SuhuTinggi = MemberSuhu;
  Sensor_Suhu(a=40, b=60, c=150);
  SuhuSangatTinggi = MemberSuhu;

  MemberAsap = 0;
  Sensor_Asap(a=0.1, b=1.4, c=2.3);
  AsapTinggi = MemberAsap;
  Sensor_Asap(a=1.4, b=2.3, c=3.5);
  AsapSedang = MemberAsap;
  Sensor_Asap(a=2.3, b=3.5, c=50);
  AsapRendah = MemberAsap;
}



void Out()
{
   if (Output < 25)
   {
    Serial.print("\tMati");
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
   }
   if (Output >=25 && Output<75)
   {
    Serial.print("\tLambat");
  digitalWrite(led1, HIGH);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
   }
   if (Output >=75 && Output<125)
   {
    Serial.print("\tCukup Lambat");
  digitalWrite(led1, LOW);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, LOW);
   }
   if (Output >=125 && Output<175)
   {
    Serial.print("\tSedang");
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, HIGH);
   }
   if (Output >=175 && Output<225)
   {
    Serial.print("\tCukup Cepat");
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, LOW);
   }
   if (Output >=225)
   {
    Serial.print("\tCepat");
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
   }
}

void Asap()
{
  int adc1 = ads.readADC_SingleEnded(1);
  sensor_volt=(float)adc1 * 1 / 65536;
  RS_gas = (1-sensor_volt)/sensor_volt;

  //Replace The name "R0" with the value of R0 in the demo of first test    -*/
  smoke = RS_gas/9.81; // ratio = R5/R0
    /*--------------------------------------------*/
delay(1000);

}

void Suhu()
{

  adc0 = ads.readADC_SingleEnded(0); 
  temp = adc0 * 1024 / 65536;
}

void kirim() {
   Serial.println(antares.storeData(projectName, deviceName1, (String)SensorSuhu, "celcius"));
   Serial.println(antares.storeData(projectName, deviceName2, (String)SensorAsap, "ppm"));
   Serial.println(antares.storeData(projectName, deviceName3, (String)pwm, "volt"));
}

