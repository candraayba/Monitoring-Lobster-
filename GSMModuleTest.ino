#include <Wire.h>
#include "SIM900.h"
#include <SoftwareSerial.h>o
#include "sms.h"
SMSGSM sms;
#include <LiquidCrystal_SR_LCD3.h>
//#include <SoftwareSerial.h>
//SoftwareSerial Serial(2, 3); // RX, TX 
#define SensorPin A0
#define Offset 0.00            //deviation compensate
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex = 0;
boolean started=false;
char smsbuffer[160];
char n[20];
const int pingPin = 7; 
char* nomor= "08889429680";
float tempC;
int uhus;
int phmeter = A0;
int tempPin=A2;
int salinity = A3;
int val;
int phint;
char position;
char phone_number[20]; // array for the phone number string
char sms_text[200];
String datasms="";
long duration, cm;
boolean statping=false, statldr=false, statph=false, statkirim=false;
static float pHValue, voltage;
String dataping = "";
String datasuhu = "";
String datagaram = "";
String dataph = "";

void setup() 
{
  //analogReference(DEFAULT);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println("LOBSTER");
  if (gsm.begin(2400)) {
    Serial.println("\nstatus=READY");
    started=true;
  } 
  else Serial.println("\nstatus=IDLE");
  if(started) {
    //Enable this two lines if you want to send an SMS.
    if (sms.SendSMS(nomor, "System has been activated"))
      Serial.println("\nSMS sent OK");
  }
  while (!Serial) 

  {
    ;  // wait for serial port to connect. Needed for Leonardo only 
  }
  //Serial.println("Goodnight moon!");
  // set the data rate for the SoftwareSerial port
  Serial.begin(9600); 
}
void loop() {
  ceksms();
  bacaping();
  bacaph();
  data();
  
  
}
void ceksms(){
  position = sms.IsSMSPresent(SMS_UNREAD);
  if (position) {
    sms.GetSMS(position, phone_number, sms_text, 200);
    sms.DeleteSMS(position);
  }
  datasms=String(sms_text);
  Serial.println(datasms);
  datasms.trim();
  delay(500);
}
void kirim(char* pesan){
  if(statkirim==true){
    Serial.print("Kirim SMS = ");
    Serial.println(pesan);
    if(started) {
      if (sms.SendSMS(nomor, pesan))
        Serial.println("\nSMS sent OK");
    }
  }
  statkirim=false;
}
void bacaping(){
  pinMode(pingPin,OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin,HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin,LOW);
  pinMode(pingPin,INPUT);
  duration = pulseIn(pingPin,HIGH);
  cm=microsecondsToCentimeters(duration);
  Serial.println();
  Serial.print("Ketinggian air: ");
  Serial.print(cm);
  Serial.println(" cm, ");
  Serial.println();
  if(cm>20){
    if(statping==false){
      statkirim=true;
    }
    Serial.println("Air Kurang");
    statping=true;
    kirim("Volume Air Kurang");
  }
  if(cm<=20){
    if(statping==true){
      statkirim=true;
    }
    Serial.println("Air Pas");
    statping=false;
    kirim("Volume Air Udah Pas");
  }
  delay(100);
}
void bacaph(){
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue, voltage;
  Serial.println();
  Serial.println("Mengukur Kadar");
  Serial.println("pH");
  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = analogRead(SensorPin);
    if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
    pHValue = 3.5 * voltage + Offset;
    samplingTime = millis();
  }
  if (millis() - printTime > printInterval) {
    printTime = millis();
    Serial.println();
    Serial.println("Kadar pH Air :");
    Serial.println(pHValue);
    if (pHValue < 6.00) {
      if(statph==false){
        statkirim=true;
      }
      statph=true;
      kirim("Kadar pH Asam");
      Serial.print("Kadar pH Asam");
    }
    else if (pHValue > 7.00) {
      if(statph==false){
        statkirim=true;
      }
      statph=true;
      kirim("Kadar pH Basa");
      Serial.println("Kadar pH Basa :");
    }
    else {
      if(statph==true){
        statkirim=true;
      }
      statph=false;
      kirim("Kadar pH Pas");
    }
  }
}
void data()
{
  static float pHValue, voltage;
  long duration, cm;
  pinMode(pingPin, OUTPUT); 
  digitalWrite(pingPin, LOW); 
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH); 
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);
  cm = microsecondsToCentimeters(duration);
  dataping=String(cm, HEX); 
  Serial.print(String(255, HEX));//header1
  Serial.print(String(255, HEX));
  Serial.print(String(255, HEX));//header2
  Serial.print(String(255, HEX));

  if (dataping.length() == 1) { //Data Ping
    Serial.print(0); 
    Serial.print(0);
    Serial.print(dataping); 
    Serial.print(dataping); 
  }
  else { 
    Serial.print(dataping); 
    Serial.print(dataping); 
  }
  suhu();
  garam();
  tampilph();
  int checksum = 255^255 ^ cm ^ uhus ^ val ^ phint ;
  String datachecksum= String (checksum, HEX);

  if (datachecksum.length() == 2) { //Data Ping
    Serial.print(0);
    Serial.print(0);
    Serial.print(datachecksum); 
    Serial.print(datachecksum);
  }



  if (datachecksum.length() == 3) { //Data Ping
    Serial.print(datachecksum); 
    Serial.print(datachecksum);
  }

  Serial.println(); 
  Serial.println("cm");
  Serial.println(cm);
  Serial.println("Suhu :");
  Serial.println(uhus);
  Serial.println("Suhu :");
  Serial.println(tempC);
  Serial.println(datasuhu);
  Serial.println("garam :");
  Serial.println(val);
  Serial.println(datagaram);
  Serial.println("pH :");
  Serial.println(phint);
  Serial.println(dataph);


  Serial.println(checksum); 
  delay (1000);
}
long microsecondsToCentimeters(long microseconds) { 
  return microseconds / 29 / 2; 
}

void suhu(){
  uhus=analogRead(tempPin);
  //uhus = 10;
  tempC=uhus* 0.48828125;
  datasuhu=String(uhus, HEX);
}

void garam(){
  val=analogRead(salinity);
  datagaram=String(val, HEX);

  if (datagaram.length() == 1) { //Data suhu
    Serial.print(0);
    Serial.print(0);
    Serial.print(0); 
    Serial.print(0);
    Serial.print(datagaram); 
    Serial.print(datagaram); 
  }
  if (datagaram.length() == 2) { //Data Ping
    Serial.print(0);
    Serial.print(0);
    Serial.print(datagaram); 
    Serial.print(datagaram);
  }

  if (datagaram.length() == 3) { //Data Ping
    Serial.print(datagaram); 
    Serial.print(datagaram);
  }

}

void tampilph(){

  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  Serial.println();
  Serial.println("Mengukur Kadar");
  Serial.println("pH");

  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = analogRead(SensorPin);
    if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLenth);
    pHValue = 3.5 * voltage* 5.0 / 1024 + Offset;
    // place = pHValue;
    samplingTime = millis();

    phint=int (voltage)*3.5*5.0/1024;

    dataph=String(int (voltage) , HEX);
    if (millis() - printTime > printInterval) {
      printTime = millis();

    }
  }



  if (dataph.length() == 1) { //Data suhu
    Serial.print(0);
    Serial.print(0);
    Serial.print(0); 
    Serial.print(0);
    Serial.print(dataph); 
    Serial.print(dataph); 
  }
  if (dataph.length() == 2) { //Data Ping
    Serial.print(0);
    Serial.print(0);
    Serial.print(dataph); 
    Serial.print(dataph);
  }


  if (dataph.length() == 3) { //Data Ping 

    Serial.print(dataph); 
    Serial.print(dataph); 

  }

  if (millis() - printTime > printInterval) {
    printTime = millis();


  }
}

double avergearray(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0) {
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if (number < 5) { //less than 5, calculated directly statistics
    for (i = 0; i < number; i++) {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  } 
  else {
    if (arr[0] < arr[1]) {
      min = arr[0]; 
      max = arr[1];
    }
    else {
      min = arr[1]; 
      max = arr[0];
    }
    for (i = 2; i < number; i++) {
      if (arr[i] < min) {
        amount += min;      //arr<min
        min = arr[i];
      } 
      else {
        if (arr[i] > max) {
          amount += max;  //arr>max
          max = arr[i];
        } 
        else {
          amount += arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount / (number - 2);
  }//if
  return avg;
}




//
//
//

