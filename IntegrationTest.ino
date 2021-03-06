#include <Wire.h>
#include <LiquidCrystal_SR_LCD3.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX, TX 
#define SensorPin A0
#define Offset 0.00            //deviation compensate
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex = 0;

const int pingPin = 7; 
float tempC;
int uhus;
int phmeter = A0;
int tempPin=A2;
int salinity = A3;
int val;
int phint;

static float pHValue, voltage;
String dataping = "";
String datasuhu = "";
String datagaram = "";
String dataph = "";

void setup() 
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  while (!Serial) 
  {
    ;  // wait for serial port to connect. Needed for Leonardo only 
  }
  
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600); 
}

void loop() {
  data();
}

// Proses data yang ditampilkan dan mengirim sms
void data()
{
  // variable init
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
  mySerial.print(String(255, HEX));//header1
  Serial.print(String(255, HEX));
  mySerial.print(String(255, HEX));//header2
  Serial.print(String(255, HEX));

  if (dataping.length() == 1) { //Data Ping
    mySerial.print(0); 
    Serial.print(0);
    mySerial.print(dataping); 
    Serial.print(dataping); 
  }
  else { 
    mySerial.print(dataping); 
    Serial.print(dataping); 
  }
  
  // Panggil function
  suhu();
  garam();
  tampilph();
  
  // Periksa checksum 
  int checksum = 255^255 ^ cm ^ uhus ^ val ^ phint ;
  String datachecksum= String (checksum, HEX);

  if (datachecksum.length() == 2) { //Data Ping
    mySerial.print(0);
    Serial.print(0);
    mySerial.print(datachecksum); 
    Serial.print(datachecksum);
  }

  if (datachecksum.length() == 3) { //Data Ping
    mySerial.print(datachecksum); 
    Serial.print(datachecksum);
  }

  // Tampilkan daTA
  Serial.println(); 
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

// Untuk microseconds
long microsecondsToCentimeters(long microseconds) { 
  return microseconds / 29 / 2; 
}

// Membaca suhu dari LM35
void suhu(){
  uhus=analogRead(tempPin);
  //uhus = 10;
  tempC=uhus* 0.48828125;
  datasuhu=String(uhus, HEX);

  // Membuat checksum
  if (datasuhu.length() == 1) { //Data suhu
    mySerial.print(0);
    Serial.print(0);
    mySerial.print(0); 
    Serial.print(0);
    mySerial.print(datasuhu); 
    Serial.print(datasuhu); 
  }
  if (datasuhu.length() == 2) { //Data Ping
    mySerial.print(0);
    Serial.print(0);
    mySerial.print(datasuhu); 
    Serial.print(datasuhu);
  }

  if (datasuhu.length() == 3) { //Data Ping 

    mySerial.print(datasuhu); 
    Serial.print(datasuhu); 
  }
}

// Membaca kadar garam 
void garam(){
  val=analogRead(salinity);
  datagaram=String(val, HEX);

    // Membuat checksum
   if (datagaram.length() == 1) { //Data suhu
    mySerial.print(0);
    Serial.print(0);
    mySerial.print(0); 
    Serial.print(0);
    mySerial.print(datagaram); 
    Serial.print(datagaram); 
  }
  if (datagaram.length() == 2) { //Data Ping
    mySerial.print(0);
    Serial.print(0);
    mySerial.print(datagaram); 
    Serial.print(datagaram);
  }

  if (datagaram.length() == 3) { //Data Ping
    mySerial.print(datagaram); 
    Serial.print(datagaram);
  }

}

// Membaca pH air
void tampilph(){

  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();


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
  }

  // Membuat checksum
  if (dataph.length() == 1) {
    mySerial.print(0);
    Serial.print(0);
    mySerial.print(0); 
    Serial.print(0);
    mySerial.print(dataph); 
    Serial.print(dataph); 
  }
  if (dataph.length() == 2) {
    mySerial.print(0);
    Serial.print(0);
    mySerial.print(dataph); 
    Serial.print(dataph);
  }

  if (dataph.length() == 3) { 

    mySerial.print(dataph); 
    Serial.print(dataph); 

  }

  if (millis() - printTime > printInterval) {
    printTime = millis();


  }
}

// library bawaan untuk membaca sensor pH
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








