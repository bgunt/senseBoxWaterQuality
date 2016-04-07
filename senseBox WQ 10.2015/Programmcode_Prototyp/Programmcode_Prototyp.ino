/*senseBox WaterQuality:
 * Sensors: - Atlas Scientific DO Probe & EZO Circut - UART
 *          - Atlas Scientific pH Probe & EZO Circut - UART
 *          - DS18B20 Temperature                    - Digital
 *          - DIY Conductivity                       - Analog
 *          - RTC RV8523                             - I2C
 *          - SeeedGrayOLED                          - I2C
 */
//SD
#include <SdFat.h>
#include <SPI.h>
#define serialSD 4

File file;
String nameOfFile;

boolean firstTime = true;

SdFat SD;

//RTC
#include <RV8523.h>
#include <Wire.h>

RV8523 rtc;

uint8_t sec, min, hour, day, month;
uint16_t year;

//OLED-Display
#include <Wire.h>
#include <SeeedGrayOLED.h>


//Atlas Scientific DO&pH
#include <SoftwareSerial.h>           

String output = "";


SoftwareSerial sSerial(11, 10);    
int s0 = 7;                         
int s1 = 6;                         
int enable_1 = 5;              
int enable_2 = 4;            


char DO[30];                
char pH[30];
byte sensor_bytes_receivedDO = 0;     
byte sensor_bytes_receivedpH = 0;
char *channel;                      
char *cmd;                          
int cases = 1;
unsigned long starttime;
unsigned long sampletime_ms = 5000;

//DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 22
OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);
char tempString[20];

//Conductivity
float Quellspannung = 5.0;
int AnalogPin = 8;
int R1 = 150.0; //Wert des bekannten Widerstands (150 Ohm)
long Messwert;
float SpannungR2; //Spannung über dem zu messenden Widerstand
float Widerstand;
float Conductivity;
 char condString[20];

void setup() {
//RTC startup
  rtc.start();
  rtc.batterySwitchOverOn();

  //SD Startup
  SD.begin(serialSD);

  //OLED-Startup
  Wire.begin();
  SeeedGrayOled.init();             
  SeeedGrayOled.clearDisplay();     
  SeeedGrayOled.setNormalDisplay(); 
  SeeedGrayOled.setVerticalMode();  

  for (char i = 0; i < 12 ; i++)
  {
    SeeedGrayOled.setTextXY(0, 0); 
    SeeedGrayOled.setGrayLevel(10); 

  }

  //Atlas Scientific startup
  pinMode(s0, OUTPUT);             
  pinMode(s1, OUTPUT);             
  pinMode(enable_1, OUTPUT);       
  pinMode(enable_2, OUTPUT);       

  Serial.begin(9600);              
  sSerial.begin(9600);            
  //DS18B20 startup
  sensors.begin();

}


void loop() {
  switch (cases) {
    
    case 1:
    output ="";
    RTC();
    break;

    case 2:
    DOxy();
    break;

    case 3:
    pHVal();
    break;

    case 4:
    Temperature();
    break;

    case 5:
    Cond();
    break;

    case 6:
    Print();
    break;

    case 7:
    //Prepare filename and open file on SD
    char charFileName[nameOfFile.length()+1]; 
    nameOfFile.toCharArray(charFileName, sizeof(charFileName));
    file = SD.open(charFileName, FILE_WRITE);
    writeData(output); //write the datastring to the sd
    break;
    
  
}
if (cases == 7) {
    cases = 1;
  }
  else
    cases ++;
}



void RTC(){
      rtc.get(&sec, &min, &hour, &day, &month, &year);

        output += day;
        output += ".";
        output += month;
        output += ".";
        output += year;
        output += ";";
        output += hour;
        output += ":";
        output += min;
        output += ":";
        output += sec;
        output += ";";
}
//---------------------
void DOxy(){
  channel = "1";          
      cmd = "r";                  
      open_channel();                               
      if (cmd != 0) {                               
        sSerial.print(cmd);                        
        sSerial.print("\r");
      }

      if (sSerial.available() > 0) {                 
        sensor_bytes_receivedDO = sSerial.readBytesUntil(13, DO, 30);
        DO[sensor_bytes_receivedDO] = 0;         
      }
      output += DO;
      output +=";";
      while ((millis() - starttime) <= sampletime_ms);
      starttime = millis();
}
//---------------------
void pHVal(){
  channel = "2";          
      cmd = "r";                  
      open_channel();                               
      if (cmd != 0) {                               
        sSerial.print(cmd);                       
        sSerial.print("\r");
      }

      if (sSerial.available() > 0) {                 
        sensor_bytes_receivedpH = sSerial.readBytesUntil(13, pH, 30);
        pH[sensor_bytes_receivedpH] = 0; 
      }

      output += pH;
      output +=";";
      
      while ((millis() - starttime) <= sampletime_ms);
      starttime = millis();
}

//---------------------
void Temperature (){
  sensors.requestTemperatures(); 
      
      dtostrf(sensors.getTempCByIndex(0), 5, 2, tempString);
      while ((millis() - starttime) <= sampletime_ms);
      starttime = millis();

      output += tempString;
      output +=";";
}
//---------------------
void Cond (){
  Messwert = 0;
      for (int i = 0; i < 5; i++) {
        Messwert += analogRead(AnalogPin);
      }
      Messwert = trunc(Messwert / 5);

      
      SpannungR2 = (Quellspannung / 1023.0) * Messwert; //Spannung berechnen
      Widerstand = R1 * (SpannungR2 / (Quellspannung - SpannungR2));  //Berechnung: (R2 = R1 * (U2/U1))
      Conductivity = 1 / (Widerstand * 4);
      dtostrf(Conductivity, 8, 6, condString);

      output += condString;
      output +=";";
}

//---------------------

void Print() {
  Serial.print("pH: ");
      Serial.println(DO);
      Serial.print("DO: ");
      Serial.println(pH);
      Serial.print("Temp: ");
      Serial.println(sensors.getTempCByIndex(0));
      Serial.print("Cond: ");
      Serial.println(condString);
      Serial.print(hour);
      Serial.print(":");
      Serial.print(min);
      Serial.print(":");
      Serial.println(sec);
      Serial.println("---");

      SeeedGrayOled.setTextXY(0,0);
      SeeedGrayOled.putString("senseBox WQ");
      SeeedGrayOled.setTextXY(1, 0);
      SeeedGrayOled.putString("Temperatur:");
      SeeedGrayOled.setTextXY(2, 0);
      SeeedGrayOled.putString(tempString);
      SeeedGrayOled.putString(" C");
      SeeedGrayOled.setTextXY(3, 0);
      SeeedGrayOled.putString("pH-Wert:");
      SeeedGrayOled.setTextXY(4, 0);
      SeeedGrayOled.putString(DO);
      SeeedGrayOled.setTextXY(5, 0);
      SeeedGrayOled.putString("02-Konz.:");
      SeeedGrayOled.setTextXY(6, 0);
      SeeedGrayOled.putString(pH);
      SeeedGrayOled.putString(" mg/L");
      SeeedGrayOled.setTextXY(7, 0);
      SeeedGrayOled.putString("Leitfhgk.: ");
      SeeedGrayOled.setTextXY(8, 0);
      SeeedGrayOled.putString(condString);
      SeeedGrayOled.putString("S/cm");
}

String generateFileName(String boardID)
{
  String fileName = String();
  unsigned int filenumber = 1;
  boolean isFilenameExisting;
  do{
    fileName = boardID;
    fileName += "-";
    fileName += filenumber;
    fileName += ".csv";
    Serial.println(fileName);
    char charFileName[fileName.length() + 1];
    fileName.toCharArray(charFileName, sizeof(charFileName));
   
    filenumber++;

    isFilenameExisting = SD.exists(charFileName);
  }while(isFilenameExisting); 
  
   Serial.print("Generated filename: ");
   Serial.println(fileName);
  
   return fileName; 
}

//---------------------

void writeData(String txt) {
  if (firstTime) {
      
      nameOfFile = generateFileName("a01");
      char charFileName[nameOfFile.length()+1];
      nameOfFile.toCharArray(charFileName, sizeof(charFileName));
      firstTime = false;
      file = SD.open(charFileName, FILE_WRITE);  
      if (file) {
        file.println("Date;Time;pH;DO;Temperature;Conductivity"); //write the CSV Header to the file
        file.close();
        }
  }
  // serial output
  Serial.println(txt);
  // sd card output
  file.println(txt);
  file.flush();
  SeeedGrayOled.setTextXY(10,0);
  SeeedGrayOled.putString("Save");
  SeeedGrayOled.setTextXY(10,4);
  SeeedGrayOled.putNumber(hour);
  SeeedGrayOled.putString(":");
  SeeedGrayOled.putNumber(min);
  SeeedGrayOled.putString(":");
  SeeedGrayOled.putNumber(sec);
  
}

//---------------------
void open_channel() {                        

  switch (*channel) {                        


    case '1':
      digitalWrite(enable_1, LOW);
      digitalWrite(enable_2, HIGH);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, LOW);
      break;

    case '2':
      digitalWrite(enable_1, LOW);
      digitalWrite(enable_2, HIGH);
      digitalWrite(s0, LOW);
      digitalWrite(s1, HIGH);
      break;


  }
}
