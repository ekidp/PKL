#include "AntaresESPHTTP.h" //Penyertaan library antares

#define ACCESSKEY "0af141272a490be9:9e9c25bfafa000c1" //pendefinisian kunci akses antares
#define WIFISSID "Astinet lt.15" //pendefinisian SSID Wifi untuk koneksi internet
#define PASSWORD "telkom2017" //pendefinisian password wifi

#include <Wire.h> //penyertaan library Wire untuk pendefinisian pin Rx Tx
#include <Adafruit_ADS1015.h> //penyertaan library I2C ADC Extender

Adafruit_ADS1115 ads; //penurunan kelas untuk ADC Extender

String projectName = "SmartChicken"; //deklarasi variabel untuk nama project Antares
String deviceName1 = "TemperatureSensor"; // deklarasi variabel untuk nama sensor suhu
String deviceName2 = "GasSensor"; // deklarasi variabel untuk nama sensor gas
String deviceName3 = "ExFan"; // deklarasi variabel untuk nama exhaust fan

Antares antares(ACCESSKEY); // penurunan kelas untuk Antares

int16_t adc0, adc1; // penurunan kelas untuk ADC Extender
  
int pwm; // deklarasi variabel untuk pwm exhaust fan
float temp = 0; // deklarasi variabel untuk nilai sensor suhu
float smoke = 0; // deklarasi variabel untuk nilai sensor asap
float sensor_volt; // deklarasi variabel untuk hasil tegangan sensor asap
float RS_gas; // deklarasi variabel untuk hasil RS gas sensor gas
float ratio; // deklarasi variabel untuk rasio gas pada sensor gas
int led1 = D6; // deklarasi variabel untuk pin LED 1
int led2 = D7; // deklarasi variabel untuk pin LED 2
int led3 = D8; // deklarasi variabel untuk pin LED 3

float a, b, c; // deklarasi variabel untuk threshold fuzzy
float SensorSuhu = 0, SensorAsap = 0; // deklarasi variabel untuk hasil sensor yang dimasukkan proses fuzzy
float SuhuNormal, SuhuAgakTinggi, SuhuTinggi, SuhuSangatTinggi; // deklarasi variabel untuk threshold sensor suhu
float AsapRendah, AsapSedang, AsapTinggi; // deklarasi variabel untuk nam
float MemberSuhu, MemberAsap; // deklarasi variabel untuk membership fuzzy
float hasil[11]; // deklarasi variabel untuk hasil metode fuzzy
float Min[11]; // deklarasi variabel untuk pengambilan nilai minimum antara hasil fuzzy suhu dan asap
float Output = 0; // deklarasi variabel untuk nilai output tegangan exhaust fan
int LOT=50, ALOT=100, SOT=150, ACOT=200, COT=255; // deklarasi variabel untuk threshold PWM exhaust fan

void setup() //deklarasi kelas setup
{
  Serial.begin(9600); //pengaturan kecepatan transmisi data
    Wire.begin(D2,D1); //pendifinisian pin Rx Tx untuk ADC Extender
    ads.begin(); //inisialisasi kelas ADC Extender
pinMode(led1, OUTPUT); //deklarasi pin LED 1 sebagai Output
pinMode(led2, OUTPUT); //deklarasi pin LED 2 sebagai Output
pinMode(led3, OUTPUT); //deklarasi pin LED 3 sebagai Output

    antares.setDebug(true); //mengaktifkan debug Antares
    antares.wifiConnection(WIFISSID,PASSWORD); //mengkoneksikan nodeMCU dengan Wifi
  

}

void loop() //deklarasi kelas loop
{
  
  Suhu(); //pemanggilan kelas suhu
  SensorSuhu = temp; //pemindahan nilai pada hasil perhitungan kelas suhu

  Asap(); //pemanggilan kelas asap
  SensorAsap = smoke; //pemindahan nilai pada hasil perhitungan kelas asap

  membershipfuzzy(); //pemanggilan kelas membership fuzzy
  defuzifikasi(); //pemanggilan kelas defuzifikasi
  Out(); //pemanggilan kelas out

  pwm = Output; //pemindahan nilai pada hasil kelas out
  
  //menampilkan hasil perhitungan seluruhnya
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
  //-------------------------------------------------------//
  
  kirim(); //pemanggilan kelas kirim
    
  delay(10000); //pengaturan delay perulangan
}

void defuzifikasi() //deklarasi kelas defuzifikasi
{
  //pengambilan nilai minimum dari nilai nilai membership fuzzy
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
  //-----------------------------------------------------------------------------------//
  
  //proses defuzzyfikasi ke pwm exhaust fan
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
  //--------------------------------------------------//
  
  //pengambilan nilai rata rata untuk nilai pwm exhaust fan
  Output = ((hasil[0]+hasil[1]+hasil[2]+hasil[3]+hasil[4]+hasil[5]+hasil[6]+hasil[7]+hasil[8]+
      hasil[9]+hasil[10]+hasil[11])/(Min[0]+Min[1]+Min[2]+Min[3]+Min[4]+Min[5]+
      Min[6]+Min[7]+Min[8]+Min[9]+Min[10]+Min[11]));
  
}

float Sensor_Asap(float a, float b, float c) //deklarasi kelas untuk proses perhitungan membership fuzzy sensor asap
{
  //percabangan jika nilai asap berada di garis antara titik a dan b dalam membership segitiga
  if ((SensorAsap >= a) && (SensorAsap < b)) 
  {
    MemberAsap = (SensorAsap - a) / (b - a); //mengubah nilai asap menjadi nilai fuzzy
  }
  //percabangan jika nilai asap berada di garis antara titik b dan c dalam membership segitiga
  if ((SensorAsap >= b) && (SensorAsap < c))
  {
    MemberAsap = (c - SensorAsap) / (c - b); //mengubah nilai asap menjadi nilai fuzzy
  }
  //percabangan jika nilai asap masuk dalam membersip trapesium
  if ((SensorAsap < 1.4) || (SensorAsap > 3.5))
  {
    MemberAsap = 1;
  }
  //error handling untuk mengatasi nilai di luar membership
  if ((SensorAsap > c) || (SensorAsap < a))
  {
    MemberAsap = 0;
  }
}

float Sensor_Suhu(float a, float b, float c) //deklarasi kelas untuk proses perhitungan membership fuzzy sensor suhu
{
	//percabangan jika nilai suhu berada di garis antara titik a dan b dalam membership segitiga
  if ((SensorSuhu >= a) && (SensorSuhu < b)) 
  {
    MemberSuhu = (SensorSuhu - a) / (b - a); //mengubah nilai suhu menjadi nilai fuzzy
  }
  //percabangan jika nilai suhu berada di garis antara titik b dan c dalam membership segitiga
  if ((SensorSuhu >= b) && (SensorSuhu < c))
  {
    MemberSuhu = (c - SensorSuhu) / (c - b); //mengubah nilai suhu menjadi nilai fuzzy
  }
  //percabangan jika nilai suhu masuk dalam membersip trapesium
  if ((SensorSuhu < 28) || (SensorSuhu > 60))
  {
    MemberSuhu = 1;
  }
  //error handling untuk mengatasi nilai di luar membership
  if ((SensorSuhu > c) || (SensorSuhu < a))
  { 
    MemberSuhu = 0;
  }
}

void membershipfuzzy() //deklarasi kelas membership fuzzy
{
  MemberSuhu = 0; //deklarasi variabel membership suhu
  Sensor_Suhu(a=0, b=28, c=35); //pemanggilan kelas sensor_suhu dan deklarasi threshold suhu normal
  SuhuNormal = MemberSuhu; //pemindahan nilai hasil kelas sensor_suhu ke variabel suhu normal
  Sensor_Suhu(a=28, b=35, c=40); //pemanggilan kelas sensor_suhu dan deklarasi threshold suhu cukup tinggi
  SuhuAgakTinggi = MemberSuhu; //pemindahan nilai hasil kelas sensor_suhu ke variabel suhu suhuagaktinggi
  Sensor_Suhu(a=35, b=40, c=60); //pemanggilan kelas sensor_suhu dan deklarasi threshold suhu tinggi
  SuhuTinggi = MemberSuhu; //pemindahan nilai hasil kelas sensor_suhu ke variabel suhu tinggi
  Sensor_Suhu(a=40, b=60, c=150); //pemanggilan kelas sensor_suhu dan deklarasi threshold suhu sangat tinggi
  SuhuSangatTinggi = MemberSuhu; //pemindahan nilai hasil kelas sensor_suhu ke variabel suhu sangat tinggi

  MemberAsap = 0; //deklarasi variabel membership asap
  Sensor_Asap(a=0.1, b=1.4, c=2.3); //pemanggilan kelas sensor_suhu dan deklarasi threshold asap tinggi
  AsapTinggi = MemberAsap; //pemindahan nilai hasil kelas sensor_asap ke variabel AsapTinggi
  Sensor_Asap(a=1.4, b=2.3, c=3.5); //pemanggilan kelas sensor_suhu dan deklarasi threshold asap sedang
  AsapSedang = MemberAsap; //pemindahan nilai hasil kelas sensor_asap ke variabel AsapSedang
  Sensor_Asap(a=2.3, b=3.5, c=50); //pemanggilan kelas sensor_suhu dan deklarasi threshold asap rendah
  AsapRendah = MemberAsap; //pemindahan nilai hasil kelas sensor_asap ke variabel AsapRendah
}



void Out() //deklarasi kelas output
{
   if (Output < 25) //percabangan jika exhaust fan mati
   {
   //memcetak status exhaust fan dan mematikan 3 led
    Serial.print("\tMati"); 
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
   }
   if (Output >=25 && Output<75) //percabangan jika exhaust fan Lambat
   {
   //memcetak status exhaust fan dan menghidupkan led pertama dan mematikan 2 led lain
    Serial.print("\tLambat");
  digitalWrite(led1, HIGH);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
   }
   if (Output >=75 && Output<125) //percabangan jika exhaust fan Cukup Lambat
   {
    //memcetak status exhaust fan dan menghidupkan led ke dua dan mematikan 2 led lain
    Serial.print("\tCukup Lambat");
  digitalWrite(led1, LOW);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, LOW);
   }
   if (Output >=125 && Output<175) //percabangan jika exhaust fan kecepatan Sedang
   {
   //memcetak status exhaust fan dan menghidupkan led ke tiga  dan mematikan 2 led lain
    Serial.print("\tSedang");
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, HIGH);
   }
   if (Output >=175 && Output<225) //percabangan jika exhaust fan Cukup Cepat
   {
   //memcetak status exhaust fan dan menghidupkan led pertama dan ke dua dan mematikan led 3
    Serial.print("\tCukup Cepat");
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, LOW);
   }
   if (Output >=225) //percabangan jika exhaust fan Cepat
   {
   //memcetak status exhaust fan dan menghidupkan semua led
    Serial.print("\tCepat");
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
   }
}

void Asap() //deklarasi kelas asap
{
  int adc1 = ads.readADC_SingleEnded(1); //pembacaan sensor asap MQ-2
  sensor_volt=(float)adc1 * 1 / 65536; // hasil sensor dikali dengan tegangan dan dibagi dengan resolusi ADC
  RS_gas = (1-sensor_volt)/sensor_volt; //perhitungan RS gas dengan cara 1 dikurangi hasil sensor kemudian dibagi hasi sensor itu sendiri
  smoke = RS_gas/9.81; //perhitungan hasil rasio dengan cara hasil RS gas dibagi konstanta gas pada udara normal
}

void Suhu() //deklarasi kelas suhu
{
  adc0 = ads.readADC_SingleEnded(0); //pembacaan sensor suhu LM35
  temp = adc0 * 1024 / 65536; // hasil sensor dikali dengan tegangan dan dibagi dengan resolusi ADC
}

void kirim() { //deklarasi kelas kirim
   Serial.println(antares.storeData(projectName, deviceName1, (String)SensorSuhu, "celcius")); //pemanggilan methode store data dan pendefinisian nama project, nama perangkat, nilai sensor suhu dan satuan
   Serial.println(antares.storeData(projectName, deviceName2, (String)SensorAsap, "ppm")); //pemanggilan methode store data dan pendefinisian nama project, nama perangkat, nilai sensor asap dan satuan
   Serial.println(antares.storeData(projectName, deviceName3, (String)pwm, "volt")); //pemanggilan methode store data dan pendefinisian nama project, nama perangkat, nilai pwn exhaust fan dan satuan
}

